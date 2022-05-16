#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 400532
#define QUANTIZE_SAMPLE 80107
#define LINEAR_SAMPLE 400535

#define HN 401407
#define H_QUANTIZE_SAMPLE 80282
#define H_LINEAR_SAMPLE 401410

// FFMPEG --> https://github.com/FFmpeg/FFmpeg
// Useful links --> https://batchloaf.wordpress.com/2017/02/10/a-simple-way-to-read-and-write-audio-and-video-files-in-c-using-ffmpeg/


// Wav file has 16 bit sample inside of it so.
// Maximum quantized number will be 2^15 (Because first bit will be sign bit)  

int16_t const maxNumber = pow(2, 15);

float const constant = pow(2, -7); 



void createCarrierSignal();
void readAudioFile();
void changeAudioFile();

int16_t findMean(int16_t first, int16_t second, int16_t third, int16_t fourth, int16_t fifth);
int16_t findBiggest(int16_t first, int16_t second, int16_t third, int16_t fourth, int16_t fifth);
int16_t findSlope(int16_t first, int16_t fifth);


void createCsvFile(int16_t* voiceArray);

float pcmModulation(float number);
int16_t decoding(float number);

int main()
{
	//createCarrierSignal();
    //readAudioFile();
    changeAudioFile();
    return 0;
}


void readAudioFile(){
	// Create a 20 ms audio buffer (assuming Fs = 44.1 kHz)
    int16_t buf[N] = {0}; // buffer
    int n;                // buffer index
     
    // Open WAV file with FFmpeg and read raw samples via the pipe.
    FILE *pipein;
    pipein = popen("ffmpeg -i monoTen.wav ", "r");
    fread(buf, 2, N, pipein);
    pclose(pipein);

    // Print the sample values in the buffer to a CSV file
    
    FILE *csvfile;
    csvfile = fopen("samples.csv", "w");
    for (n=0 ; n<N ; ++n) fprintf(csvfile, "%d\n", buf[n]);
    fclose(csvfile);
}

void createCarrierSignal(){
	// Create audio buffer
    int16_t buf[HN] = {0}; // buffer
    int n;                // buffer index
    double Fs = 40000;  // sampling frequency
     
    // Generate 1 second of audio data - it's just a 1 kHz sine wave
    for (n=0 ; n<HN ; ++n) buf[n] = 16383.0 * sin(n*1000.0*2.0*M_PI/Fs);
     
    // Pipe the audio data to ffmpeg, which writes it to a wav file
    FILE *pipeout;
    pipeout = popen("ffmpeg -y -f s16le -ar 40000 -ac 1 -i - carrier.wav", "w");
    fwrite(buf, 2, HN, pipeout);
    pclose(pipeout);
}

int16_t findMean(int16_t first, int16_t second, int16_t third, int16_t fourth, int16_t fifth){
	return round((first + second + third + fourth + fifth) / 5); 
}

int16_t findBiggest(int16_t first, int16_t second, int16_t third, int16_t fourth, int16_t fifth){
	int16_t biggest = 0;
	
	if(first > biggest){
		biggest = first;
	}

	if(second > biggest){
		biggest = first;
	}

	if(third > biggest){
		biggest = first;
	}

	if(fourth > biggest){
		biggest = first;
	}

	if(fifth > biggest){
		biggest = first;
	}

	return biggest;
}


int16_t findSlope(int16_t first, int16_t fifth){
	return round ((fifth - first)/5);
}

void changeAudioFile(){

	int16_t quantizeBuffer[H_QUANTIZE_SAMPLE] = {0};
	int16_t meanQuantizeBuffer[H_QUANTIZE_SAMPLE] = {0};
	int16_t biggestQuantizeBuffer[H_QUANTIZE_SAMPLE] = {0};

	float pcmBuffer[H_QUANTIZE_SAMPLE] = {0};
	float meanPcmBuffer[H_QUANTIZE_SAMPLE] = {0};
	float biggestPcmBuffer[H_QUANTIZE_SAMPLE] = {0};

	int16_t encodedBuffer[H_QUANTIZE_SAMPLE] = {0};
	int16_t meanEncodedBuffer[H_QUANTIZE_SAMPLE] = {0};
	int16_t biggestEncodedBuffer[H_QUANTIZE_SAMPLE] = {0};
	int16_t linearEncodedBuffer[H_LINEAR_SAMPLE] = {0};

	// Launch two instances of FFmpeg, on-e to read the original WAV
    // file and another to write the modified WAV file. In each case,
    // data passes between this program and FFmpeg through a pipe.
    FILE *pipein;

    FILE *pipeout;
    FILE *meanPipeout;
    FILE *biggestPipeout;

    FILE *encPipeout;
    FILE *encMeanPipeout;
    FILE *encBiggestPipeout;

    FILE *linearPipeout;


    // Analog to Digital Converter
    pipein  = popen("ffmpeg -i humanSounds/human.wav -f s16le -ac 1 -", "r");

    pipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanOut.wav", "w");
    meanPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanMeanOut.wav", "w");
    biggestPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanBigOut.wav", "w");
    
    encPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/enc_humanOut.wav", "w");
    encMeanPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/enc_humanMeanOut.wav", "w");
    encBiggestPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/enc_humanBigOut.wav", "w");

	linearPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/enc_humanLinearOut.wav", "w");


    // Read, modify and write one sample at a time
    int16_t sample;
    int count, k, n=0;

    int16_t first, second, third, fourth, fifth, slope = 0;
    while(1){
        count = fread(&sample, 2, 1, pipein); // read one 2-byte sample
        if (count != 1) break;
        
        //printf("\n%d", n);
        
        if( n%5 == 0){
        	first = sample;
        } else if(n%5 == 1){
        	second = sample;
        } else if( n%5 == 2){
        	third = sample;
        } else if(n%5 == 3){
        	fourth = sample;
        } else if( n%5 == 4){
        	fifth = sample;
        	k = (n-4)/5;

			// quantize number will be first sample always for this situation.
        	quantizeBuffer[k] = first;

        	// quantize number will be mean of 5 signal always for this situation. ( (x0 + x1 + x2 + x3 + x4 )/5)
			meanQuantizeBuffer[k] = findMean(first, second, third, fourth, fifth);

			// quantize number will be the biggest number in 5 signal level. biggest number >= x0 && x1 && x2 && x3 && x4
        	biggestQuantizeBuffer[k] = findBiggest(first, second, third, fourth, fifth);
        }
        n++;
    }

	//createCsvFile(quantizeBuffer);

    // Pulse Code Modulation
    for(int i = 0; i<H_QUANTIZE_SAMPLE; i++){

    	//It will be between -1V to 1V
    	pcmBuffer[i] = pcmModulation((float)quantizeBuffer[i] / maxNumber);
    	meanPcmBuffer[i] = pcmModulation((float)meanQuantizeBuffer[i] / maxNumber);
    	biggestPcmBuffer[i] = pcmModulation((float)biggestQuantizeBuffer[i] / maxNumber);
    }

	//createCsvFile(pcmBuffer);

    // Encoding

    for(int i = 0; i<H_QUANTIZE_SAMPLE; i++){
    	encodedBuffer[i] = decoding(pcmBuffer[i]);
    	meanEncodedBuffer[i] = decoding(meanPcmBuffer[i]);
    	biggestEncodedBuffer[i] = decoding(biggestPcmBuffer[i]);
    }
   
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		//printf("%d", encodedBuffer[i]);
		for(int j=0; j<5; j++){
			//quantized buffer write
			fwrite(quantizeBuffer + i, 2, 1, pipeout);
			fwrite(meanQuantizeBuffer + i, 2, 1, meanPipeout);
			fwrite(biggestQuantizeBuffer + i, 2, 1, biggestPipeout);

			//encoded buffer write
			fwrite(encodedBuffer + i, 2, 1, encPipeout);
			fwrite(meanEncodedBuffer + i, 2, 1, encMeanPipeout);
			fwrite(biggestEncodedBuffer + i, 2, 1, encBiggestPipeout);

		}
	}  

	//createCsvFile(encodedBuffer);


	printf("\nLinearQuantizeBuffer: ");
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		if(i != 0 ){ 
			slope = findSlope(encodedBuffer[i-1], encodedBuffer[i]);
			for(int j=0; j<5; j++){
				linearEncodedBuffer[i + j] = encodedBuffer[i-1] + (slope * j);
	        	fwrite(linearEncodedBuffer + i + j, 2, 1, linearPipeout);
			}
		}
	}  
	
	//createCsvFile(linearEncodedBuffer);

	// Cubic interpolation is the better solution for this -> https://en.wikipedia.org/wiki/Bicubic_interpolation

    // Close input and output pipes
    pclose(pipein); 

    pclose(pipeout);
    pclose(meanPipeout);
    pclose(biggestPipeout);

	pclose(encPipeout);
    pclose(encMeanPipeout);
    pclose(encBiggestPipeout);

	pclose(linearPipeout);
}

void createCsvFile(int16_t voiceArray[HN]){
	FILE *csvfile;
    csvfile = fopen("Encodedsamples.csv", "w");
    for (int n=0 ; n<HN ; ++n) fprintf(csvfile, "%d\n", voiceArray[n]);
    fclose(csvfile);
}

int16_t decoding(float number){
	return round(number * maxNumber);
}

float pcmModulation(float number){
	float bias = 1.0;
	number = number + bias;
	for(int16_t i = 0; i< pow(2,8); i++){
		if(number< i* constant){
			return (((i*constant) - (constant / 2)) - bias);
		}
	}
	return (1 - (constant / 2));
}
