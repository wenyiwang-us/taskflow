static int
gomp_push_task(struct gomp_task *task)
{
	struct gomp_thread *thr = gomp_thread();
	struct gomp_team *team = thr->ts.team;
	unsigned long gtid = (unsigned long)omp_get_thread_num();

	// Check if deque is full
	int num_tries = 0;
	unsigned long last_q = thr->last_q;
	
	// NARP changes
	struct gomp_thread *target_thr;
	unsigned long target_tid;

#ifdef XGOMP_NARP
	struct wsd *wsd = &thr->wsd;
	if(wsd->flag == NARP_IDLE){
#endif // XGOMP_NARP
		/* Not using redirect push here */
		target_tid = gtid + last_q; // starting target tid
		target_tid = (target_tid > team->nthreads - 1) ? (target_tid - team->nthreads) : target_tid;
#ifdef XGOMP_NARP
	}else{
		/**
		 * Now the last_q starts from the redirect_tid.
		 * TODO: An alternative is to not update last_q at the end of this function.
		 */
		target_tid = (unsigned long) wsd->redirect_tid;
		last_q = target_tid < gtid ? target_tid - gtid + team->nthreads : target_tid - gtid;
	}
#endif // XGOMP_NARP
		// TODO: this is other ways to make sure it is serial
	if (team->nthreads <= 1)
		target_thr = thr;
	else
		target_thr = thr->thread_pool->threads[target_tid]; //ww: does this pointer the same across threads? - seems so

	/* XQueue: check target queue availability*/
	while (target_thr->td_task_q[last_q]->td_deque[target_thr->td_task_q[last_q]->td_deque_head] != NULL){
		num_tries++;
		if (num_tries < 25)
			continue;

/* PSTATS: task not pushed because target queue full */
#ifdef XGOMP_PSTATS
#ifdef XGOMP_NARP
	/**
	 * Record static push/not push when NA-RP is idle
	 * Else it is considered as a not-pushed task by the DLB.
	 * Current request is also considered as failed if it steals none.
	*/
	if(wsd->flag == NARP_IDLE){
		thr->pstats[STATS_STATIC_NTASK_NOT_PUSHED]++;
	}else{
		thr->pstats[STATS_DLB_NTASK_NOT_PUSHED]++;
		if(wsd->nredirect <= 0){
			// For NA-RP, both no-steal and target-full results from target is full,
			// we keep no-steal as redundancy for sanity check
			thr->pstats[STATS_DLB_NREQ_HAS_NO_STEAL]++;
			thr->pstats[STATS_DLB_NREQ_TARGET_FULL]++;
		}
	}
		
#else
		thr->pstats[STATS_STATIC_NTASK_NOT_PUSHED]++;
#endif // XGOMP_NARP
#endif // XGOMP_PSTATS

#ifdef XGOMP_NARP
		/* Reset flags for a new NARP because target is full */
		wsd->flag = NARP_IDLE;
		wsd->redirect_tid = -1;
		wsd->nredirect = 0;
		wsd->round++;
#endif // XGOMP_NARP
		// We move to next queue in case target is full, casuing sequential execution of current thread.
		if(target_tid == gtid){
			if (thr->num_queues > 1){
				last_q++;
				if (last_q < thr->num_queues)
					thr->last_q = last_q;
				else
					thr->last_q = 0;
				}
		}

		return TASK_NOT_PUSHED;
	}

	/* XQueue: task push routine */
	struct gomp_taskq *task_q = target_thr->td_task_q[last_q];
#if defined(XGOMP_PLOG) || defined(XGOMP_PSTATS)
	task_q->nin++; // Doesn't matter if it is reordered by the CPU/compiler
#endif
	task_q->td_deque[task_q->td_deque_head] = task;
	task_q->td_deque_head = (task_q->td_deque_head + 1) & TASK_DEQUE_MASK(thr);
	target_thr->last_q_accessed = last_q;

/* PSTATS: task pushed */
#ifdef XGOMP_PSTATS
#ifdef XGOMP_NARP
	// NA-RP
	unsigned long mytid = (unsigned long) thr->ts.team_id;
	unsigned int numa_start = wsd->leader;
	unsigned int numa_end = numa_start + wsd->ncores_numa;
	if(wsd->flag == NARP_IDLE)
		thr->pstats[STATS_STATIC_NTASK_PUSHED]++; // record na-rp's static task push
	else{
		thr->pstats[STATS_DLB_NTASK_PUSHED]++; // record na-rp's dlb task push
		// TODO: also decide task locality of this stolen task
		if(target_tid >= numa_start && target_tid < numa_end)
		{
			if(target_tid == mytid)
				thr->pstats[STATS_DLB_NTASK_STOLEN_SELF]++;
			else
				thr->pstats[STATS_DLB_NTASK_STOLEN_LOCAL]++;
		}
		else
		{
			thr->pstats[STATS_DLB_NTASK_STOLEN_REMOTE]++;
		}
	}
		
#else
	// XGOMPTB
	thr->pstats[STATS_STATIC_NTASK_PUSHED]++; // static task push
#endif // XGOMP_NARP
#endif // XGOMP_PSTATS
		
#ifdef XGOMP_NARP
	/**
	 * Reset the flag, redirect_tid and nredirects if
	 * - The task is pushed successfully
	 * - Currently we are redirecting tasks
	 * - Including this task, nredirect >= nsteals
	 * We do
	 * - Reset everything
	 * - Increment the round since we fullfilled this round
	 */
	if(wsd->flag == NARP_HANDLING_REQ && ++wsd->nredirect >= wsd->nsteals)
	{
		wsd->flag = NARP_IDLE;
		wsd->redirect_tid = -1;
		wsd->nredirect = 0;
		wsd->round++;
		/* PSTATS: req has steal */
		#ifdef XGOMP_PSTATS
		thr->pstats[STATS_DLB_NREQ_HAS_STEAL]++;
		#endif // XGOMP_PSTATS
	}
#endif // XGOMP_NARP

		if (thr->num_queues > 1){
			last_q++;
			if (last_q < thr->num_queues)
				thr->last_q = last_q;
			else
				thr->last_q = 0;
		}
		return TASK_SUCCESSFULLY_PUSHED;
};

static gomp_task_t* 
gomp_remove_my_task()
{
	gomp_task_t *task;
	struct gomp_thread *thr = gomp_thread();

	if (thr->td_task_q[0]->td_deque[thr->td_task_q[0]->td_deque_tail] == NULL)
		return NULL;
	task = (gomp_task_t *) thr->td_task_q[0]->td_deque[thr->td_task_q[0]->td_deque_tail];
	thr->td_task_q[0]->td_deque[thr->td_task_q[0]->td_deque_tail] = NULL;
	thr->td_task_q[0]->td_deque_tail = (thr->td_task_q[0]->td_deque_tail + 1) & TASK_DEQUE_MASK(thr);
#if defined(XGOMP_PLOG) || defined(XGOMP_PSTATS)
	thr->td_task_q[0]->nout++;
#endif
	return task;
};

static gomp_task_t*
gomp_remove_aux_task(unsigned long *last_qid)
{
	gomp_task_t *task;
	struct gomp_thread *thr = gomp_thread();

	task = NULL;
	struct gomp_taskq *task_q= NULL;
	/* Try pop from last accessed queue */
	if(thr->last_q_accessed > 0){
		task_q = thr->td_task_q[thr->last_q_accessed];
		if (task_q->td_deque[task_q->td_deque_tail] != NULL){
			task = (gomp_task_t *) task_q->td_deque[task_q->td_deque_tail];
			task_q->td_deque[task_q->td_deque_tail] = NULL;
			task_q->td_deque_tail = (task_q->td_deque_tail + 1) & TASK_DEQUE_MASK(thr);
			*last_qid = thr->last_q_accessed;
		}
			
	}
	/* Try pop from last queue */
	if(task == NULL){
		for(unsigned long queue_id = *last_qid; queue_id > 0; queue_id --){
			task_q = thr->td_task_q[queue_id];
			if (task_q->td_deque[task_q->td_deque_tail] != NULL){
				task = (gomp_task_t *) task_q->td_deque[task_q->td_deque_tail];
				task_q->td_deque[task_q->td_deque_tail] = NULL;
				task_q->td_deque_tail = (task_q->td_deque_tail + 1) & TASK_DEQUE_MASK(thr);
				*last_qid = queue_id;
				break;
			}
		}
	}
	/* Try pop from queue starting from the first queue */
	if(task == NULL){
		for(unsigned long queue_id = thr->num_queues - 1; queue_id > *last_qid; queue_id --){
			task_q = thr->td_task_q[queue_id];
			if (task_q->td_deque[task_q->td_deque_tail] != NULL){
				task = (gomp_task_t *) task_q->td_deque[task_q->td_deque_tail];
				task_q->td_deque[task_q->td_deque_tail] = NULL;
				task_q->td_deque_tail = (task_q->td_deque_tail + 1) & TASK_DEQUE_MASK(thr);
				*last_qid = (queue_id);
				break;
			}
		}
	}
#if defined(XGOMP_PLOG) || defined(XGOMP_PSTATS)
	if(__builtin_expect(task != NULL,1))
		task_q->nout++;
#endif
	return task;
};
