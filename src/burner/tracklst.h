#ifndef TRACKLST_H
#define TRACKLST_H

extern int nTrackCnt, nSectCnt;
extern int nCurrentTrack, nCurrentSect;
extern bool bNoSect;

void sendSoundCode(UINT16 nCode);

int getTrackCode(int nSect, int nTrack);
TCHAR* getTrackDesc(int nSect, int nTrack, bool bIsSect);

void playNextTrack();
void playPreviousTrack();
void playCurrentTrack();

void selectNextSect();
void selectPreviousSect();
void setCurrentSect();

int parseTracklist();
void resetTracklist();

#endif
