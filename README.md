# This repo created for Communication 2 PCM Modulation project.

Bariscan Kurtkaya
17014090

# Algorithm

- Firstly, It's geting raw wav data with ffmpeg library. [Input Signal]
- Secondly, It samples the 40K HZ data with 8K Samples per/sec.
- After that, I quantized all the data and implement PCM Modulation.
- Then, I decode the pcm modulated data. [Decoded Output Signal].
- For error correction I used Linear regression method and try to approximate to real values. [Linear Regression Output Signal]

# Input Signal

<img src="./Data/inputSignal.png" alt="Input Signal" width="600" height="300">

# Decoded Output Signal

<img src="./Data/decodedOutputSignal.png" alt="Decoded Signal" width="600" height="300">

# Linear Regression Output Signal

<img src="./Data/linearRegressionOutputSignal.png" alt="Linear Regression Signal" width="600" height="300">
