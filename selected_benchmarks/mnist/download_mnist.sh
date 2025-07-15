#!/bin/bash

# Download MNIST dataset files from Kaggle
# Note: You may need to authenticate with Kaggle first using kaggle CLI
# or download manually from: https://www.kaggle.com/datasets/hojjatk/mnist-dataset

echo "Downloading MNIST dataset files..."

# Create a temporary directory for downloads
mkdir -p temp_downloads
cd temp_downloads

# Download the files using wget
# Note: These URLs are direct download links from Kaggle
# You may need to replace these with actual download URLs after logging into Kaggle

echo "Downloading t10k-images-idx3-ubyte..."
wget -O t10k-images-idx3-ubyte "https://www.kaggle.com/api/v1/datasets/download/hojjatk/mnist-dataset?file=t10k-images-idx3-ubyte" --user-agent="Mozilla/5.0" --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header="Accept-Language: en-US,en;q=0.5" --header="Accept-Encoding: gzip, deflate" --header="Connection: keep-alive"

echo "Downloading t10k-labels-idx1-ubyte..."
wget -O t10k-labels-idx1-ubyte "https://www.kaggle.com/api/v1/datasets/download/hojjatk/mnist-dataset?file=t10k-labels-idx1-ubyte" --user-agent="Mozilla/5.0" --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header="Accept-Language: en-US,en;q=0.5" --header="Accept-Encoding: gzip, deflate" --header="Connection: keep-alive"

echo "Downloading train-images.data..."
wget -O train-images.data "https://www.kaggle.com/api/v1/datasets/download/hojjatk/mnist-dataset?file=train-images.data" --user-agent="Mozilla/5.0" --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header="Accept-Language: en-US,en;q=0.5" --header="Accept-Encoding: gzip, deflate" --header="Connection: keep-alive"

echo "Downloading train-labels.data..."
wget -O train-labels.data "https://www.kaggle.com/api/v1/datasets/download/hojjatk/mnist-dataset?file=train-labels.data" --user-agent="Mozilla/5.0" --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header="Accept-Language: en-US,en;q=0.5" --header="Accept-Encoding: gzip, deflate" --header="Connection: keep-alive"

# Move files to parent directory
mv *.data ../
mv *.ubyte ../

# Clean up temporary directory
cd ..
rmdir temp_downloads

echo "Download complete! Files are now in the current directory."
echo "Files downloaded:"
echo "  - t10k-images-idx3-ubyte"
echo "  - t10k-labels-idx1-ubyte"
echo "  - train-images.data"
echo "  - train-labels.data" 
