#ifndef _KAILLERADLL_H
#define _KAILLERADLL_H

extern BOOL Kaillera_Is_Running;
extern int Kaillera_Players;
extern unsigned int Kaillera_Counter;

int LoadDllKaillera();
BOOL IsKailleraDllLoaded();
int UnloadDllKaillera();

/* Kaillera 0.84 spec */
typedef struct
{
  char *appName;
  char *gameList;

  int (WINAPI *gameCallback)(char *game, int player, int numplayers);

  void (WINAPI *chatReceivedCallback)(char *nick, char *text);
  void (WINAPI *clientDroppedCallback)(char *nick, int playernb);

  void (WINAPI *moreInfosCallback)(char *gamename);
} kailleraInfos;

int kailleraGetVersion(char *version);
int kailleraInit();
int kailleraShutdown();
int kailleraSetInfos(kailleraInfos *infos);
int kailleraSelectServerDialog(HWND parent);
int kailleraModifyPlayValues(void *values, int size);
int kailleraChatSend(char *text);
int kailleraEndGame();

#endif // _KAILLERADLL_H