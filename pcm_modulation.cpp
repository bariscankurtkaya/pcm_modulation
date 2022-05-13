#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 400532
#define QUANTIZE_SAMPLE 80107
#define LINEAR_SAMPLE 400535

#define HN 407078
#define H_QUANTIZE_SAMPLE 81416
#define H_LINEAR_SAMPLE 407080

// Wav file has 16 bit sample inside of it so.
// Maximum quantized number will be 2^15 (Because first bit will be sign bit)  

int16_t const maxNumber = pow(2, 15);  

void createAudio();
void readAudioFile();
void changeAudioFile();

int main()
{
	//createAudio();
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

void createAudio(){
	// Create audio buffer
    int16_t buf[N] = {0}; // buffer
    int n;                // buffer index
    double Fs = 44100.0;  // sampling frequency
     
    // Generate 1 second of audio data - it's just a 1 kHz sine wave
    for (n=0 ; n<N ; ++n) buf[n] = 16383.0 * sin(n*1000.0*2.0*M_PI/Fs);
     
    // Pipe the audio data to ffmpeg, which writes it to a wav file
    FILE *pipeout;
    pipeout = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - beep.wav", "w");
    fwrite(buf, 2, N, pipeout);
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
	int16_t linearQuantizeBuffer[H_LINEAR_SAMPLE] = {0};


	// Launch two instances of FFmpeg, on-e to read the original WAV
    // file and another to write the modified WAV file. In each case,
    // data passes between this program and FFmpeg through a pipe.
    FILE *pipein;
    FILE *pipeout;
    FILE *meanPipeout;
    FILE *biggestPipeout;
    FILE *linearPipeout;

    pipein  = popen("ffmpeg -i humanSounds/human.wav -f s16le -ac 1 -", "r");
    pipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanOut.wav", "w");
    meanPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanMeanOut.wav", "w");
    biggestPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanBigOut.wav", "w");
	linearPipeout = popen("ffmpeg -y -f s16le -ar 40707 -ac 1 -i - humanSounds/humanLinearOut.wav", "w");
    // Read, modify and write one sample at a time
    int16_t sample;
    int count, k, n=0;

    int16_t first, second, third, fourth, fifth, slope = 0;
    while(1)
    {
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

        	// quantize number will be 4th sample always for this situation.
        	quantizeBuffer[k] = sample;

        	// quantize number will be mean of 5 signal always for this situation. ( (x0 + x1 + x2 + x3 + x4 )/5)
			meanQuantizeBuffer[k] = findMean(first, second, third, fourth, fifth);

			// quantize number will be the biggest number in 5 signal level. biggest number >= x0 && x1 && x2 && x3 && x4
        	biggestQuantizeBuffer[k] = findBiggest(first, second, third, fourth, fifth);
        }
        n++;
    }


    printf("\nquantizeBuffer: ");

   
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		for(int j=0; j<5; j++){
			fwrite(quantizeBuffer + i, 2, 1, pipeout);
		}
	}  

	printf("\nMeanQuantizeBuffer: ");
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		for(int j=0; j<5; j++){
        	fwrite(meanQuantizeBuffer + i, 2, 1, meanPipeout);
		}
	}

	printf("\nBiggestQuantizeBuffer: ");
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		for(int j=0; j<5; j++){
        	fwrite(biggestQuantizeBuffer + i, 2, 1, biggestPipeout);
		}
	}

	printf("\nLinearQuantizeBuffer: ");
	for(int i=0; i<H_QUANTIZE_SAMPLE; i++){
		if(i != 0 ){ 
			slope = findSlope(quantizeBuffer[i-1], quantizeBuffer[i]);
			for(int j=0; j<5; j++){
				linearQuantizeBuffer[i + j] = quantizeBuffer[i-1] + (slope * j);
	        	fwrite(linearQuantizeBuffer + i + j, 2, 1, linearPipeout);
			}
		}
	}  
	
	// Cubic interpolation is the better solution for this -> https://en.wikipedia.org/wiki/Bicubic_interpolation

    // Close input and output pipes
    pclose(pipein);    
    pclose(pipeout);
    pclose(meanPipeout);
    pclose(biggestPipeout);
	pclose(linearPipeout);
}