#include <stdio.h>
#include <string.h>
#include <chrono>
#include <vector>
#include <cmath>
#include "fssimplewindow.h"
#include "yssimplesound.h"
#include "ysglfontdata.h"
#include "yspng.h"
#define M_PI 3.14159265358979323846

using namespace std;

const int window_height = 768;
const int window_width = 800;
const int num_white_keys = 21;  // 3 octaves
 int white_key_width = 35;
const int white_key_height = 250;
 int black_key_width = 20;
const int black_key_height = 160;

struct PianoKey {
    int x, y;
    bool isBlack;
    bool isPressed;
    const char* note;
    double frequency;
};

vector<PianoKey> piano_keys;
YsSoundPlayer soundPlayer;
YsSoundPlayer::SoundData wavData;
YsRawPngDecoder backgroundImg;

void InitializePianoKeys() {
    const char* white_notes[] = {"C", "D", "E", "F", "G", "A", "B"};
    const char* black_notes[] = {"C#", "D#", "F#", "G#", "A#"};
    
    // Calculate the width of each white key based on the window width
    white_key_width = window_width / num_white_keys;
    black_key_width = white_key_width * 2 / 3;

    int x = 0; // Start from the left edge of the window
    int white_index = 0;
    int black_index = 0;
    
    double base_freq = 130.81; // C3 frequency

    for (int octave = 0; octave < 3; octave++) {
        for (int i = 0; i < 7; i++) {
            double freq = base_freq * pow(2, (white_index % 12) / 12.0);
            piano_keys.push_back({x, window_height - white_key_height, false, false, white_notes[i], freq});
            
            if (i != 2 && i != 6) {
                freq = base_freq * pow(2, ((white_index + 1) % 12) / 12.0);
                piano_keys.push_back({x + white_key_width - black_key_width / 2, 
                                      window_height - white_key_height, 
                                      true, false, black_notes[black_index % 5], freq});
                black_index++;
            }
            
            x += white_key_width;
            white_index++;
        }
    }
}

void DrawPianoKeys() {
    // Draw white keys
    for (const auto& key : piano_keys) {
        if (!key.isBlack) {
            glColor3ub(key.isPressed ? 200 : 255, key.isPressed ? 200 : 255, key.isPressed ? 200 : 255);
            glBegin(GL_QUADS);
            glVertex2i(key.x, key.y);
            glVertex2i(key.x + white_key_width, key.y);
            glVertex2i(key.x + white_key_width, key.y + white_key_height);
            glVertex2i(key.x, key.y + white_key_height);
            glEnd();
            
            glColor3ub(0, 0, 0);
            glBegin(GL_LINE_LOOP);
            glVertex2i(key.x, key.y);
            glVertex2i(key.x + white_key_width, key.y);
            glVertex2i(key.x + white_key_width, key.y + white_key_height);
            glVertex2i(key.x, key.y + white_key_height);
            glEnd();

            // Draw note name
            glColor3ub(0, 0, 0);
            glRasterPos2i(key.x + white_key_width / 2 - 4, key.y + white_key_height - 20);
            YsGlDrawFontBitmap12x16(key.note);
        }
    }
    
    // Draw black keys
    for (const auto& key : piano_keys) {
        if (key.isBlack) {
            glColor3ub(key.isPressed ? 50 : 0, key.isPressed ? 50 : 0, key.isPressed ? 50 : 0);
            glBegin(GL_QUADS);
            glVertex2i(key.x, key.y);
            glVertex2i(key.x + black_key_width, key.y);
            glVertex2i(key.x + black_key_width, key.y + black_key_height);
            glVertex2i(key.x, key.y + black_key_height);
            glEnd();

            // Draw note name for black keys
            glColor3ub(255, 255, 255);
            glRasterPos2i(key.x + black_key_width / 2 - 6, key.y + black_key_height - 20);
            YsGlDrawFontBitmap10x14(key.note);
        }
    }
}


void PressKey(int keyIndex) {
    if (keyIndex >= 0 && keyIndex < piano_keys.size()) {
        piano_keys[keyIndex].isPressed = true;
    }
}

void ReleaseAllKeys() {
    for (auto& key : piano_keys) {
        key.isPressed = false;
    }
}

void DrawBackground() {
    glRasterPos2i(0, window_height - 1);  // Position bottom-left corner of the image
    glDrawPixels(backgroundImg.wid, backgroundImg.hei, GL_RGBA, GL_UNSIGNED_BYTE, backgroundImg.rgba);
}

void DisplayText(const char* text1, const char* text2, const char* text3, int ms) {
    int textLengths[3] = {(int)strlen(text1), (int)strlen(text2), (int)strlen(text3)};
    int numLines = (textLengths[0] > 0) + (textLengths[1] > 0) + (textLengths[2] > 0);

    auto startTime = chrono::high_resolution_clock::now();
    int elapsed = 0;

    while (elapsed < ms) {
        FsPollDevice();
        if (FsInkey() == FSKEY_ESC) {
            break;  // Exit display on ESC key press
        }

        auto currentTime = chrono::high_resolution_clock::now();
        elapsed = (int)chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count();

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        DrawBackground();
        
        glColor3ub(0, 0, 0);

        // Center and display each line of text
        glRasterPos2i(window_width / 2 - textLengths[0] / 2 * 16, window_height / 2 - numLines * 20 / 2);
        YsGlDrawFontBitmap16x20(text1);
        
        if (textLengths[1] > 0) {
            glRasterPos2i(window_width / 2 - textLengths[1] / 2 * 16, window_height / 2 - numLines * 20 / 2 + 25);
            YsGlDrawFontBitmap16x20(text2);
        }
        
        if (textLengths[2] > 0) {
            glRasterPos2i(window_width / 2 - textLengths[2] / 2 * 16, window_height / 2 - numLines * 20 / 2 + 50);
            YsGlDrawFontBitmap16x20(text3);
        }

        FsSwapBuffers();
        FsSleep(25);  // Add a brief pause for smoother display
    }
}


double DetectFrequency(const short* data, int numSamples, int sampleRate) {
    const int minFreq = 100;
    const int maxFreq = 1000;
    const int numFreqs = maxFreq - minFreq + 1;
    
    double maxMagnitude = 0;
    int detectedFreq = 0;
    
    for (int freq = minFreq; freq <= maxFreq; freq++) {
        double sumReal = 0, sumImag = 0;
        for (int i = 0; i < numSamples; i++) {
            double angle = 2 * M_PI * freq * i / sampleRate;
            sumReal += data[i] * cos(angle);
            sumImag += data[i] * sin(angle);
        }
        double magnitude = sqrt(sumReal * sumReal + sumImag * sumImag);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            detectedFreq = freq;
        }
    }
    
    return detectedFreq;
}

int playPiano(const char* fName) {
    YsSoundPlayer player;
    YsSoundPlayer::SoundData wav;
    player.MakeCurrent();
    player.Start();
    
    if( YSOK != wav.LoadWav(fName) ) {
        printf("Couldn't load file");
        return 1;
    }
    int play_back_rate = wav.PlayBackRate();

    player.PlayOneShot(wav);

    auto begin = chrono::high_resolution_clock::now();

    const int analysisSamples = 2048;
    vector<short> sampleBuffer(analysisSamples);

    while(YSTRUE==player.IsPlaying(wav)){
        FsPollDevice();
        int key = FsInkey();
        if( key == FSKEY_ESC ) {
            break;
        }
        auto cur = chrono::high_resolution_clock::now();  
        auto dur = cur - begin;
        double s = (double)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()/1000.0;
        if(s<0){ s = 0; }

        if (s >= 40.0) {
            break;
        }


        long long int curNumSample = wav.SecToNumSample(s, play_back_rate);
        
        // Get audio samples for analysis
        for (int i = 0; i < analysisSamples; i++) {
            sampleBuffer[i] = wav.GetSignedValueRaw(0, curNumSample + i);
        }
        
        double detectedFreq = DetectFrequency(sampleBuffer.data(), analysisSamples, play_back_rate);
        
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        DrawBackground();
        
        ReleaseAllKeys();
        
        // Find and press the key closest to the detected frequency
        if (detectedFreq > 0) {
            double minDiff = 1000000;
            int closestKey = -1;
            for (int i = 0; i < piano_keys.size(); i++) {
                double diff = std::abs(piano_keys[i].frequency - detectedFreq);
                if (diff < minDiff) {
                    minDiff = diff;
                    closestKey = i;
                }
            }
            if (closestKey != -1) {
                PressKey(closestKey);
            }
        }
        
        DrawPianoKeys();
        
        FsSwapBuffers();
        player.KeepPlaying();
        FsSleep(1);
    }

    player.End();

    return 0;
}

int main(void)
{
    FsChangeToProgramDir();
    FsOpenWindow(0, 0, window_width, window_height, 1);
    
    if (YSOK != backgroundImg.Decode("background.png")) {
        printf("Failed to load background image.\n");
        return 1;
    }
    backgroundImg.Flip();

    InitializePianoKeys();
    
    if (FsInkey() == FSKEY_ESC) return 0; 
    DisplayText("Welcome to the concert!", "", "", 3000);
    if (FsInkey() == FSKEY_ESC) return 0;
    DisplayText("Watch the piano keys play along", "with the music", "Relax and Enjoy Beethoven!", 4000);
    if (FsInkey() == FSKEY_ESC) return 0; 
    playPiano("fur_elise.wav");  
    if (FsInkey() == FSKEY_ESC) return 0; 
    DisplayText("Thank you for listening!", "", "", 3000);

    return 0;

}