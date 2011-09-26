// autofire module, added by regret
#ifndef AUTOFIRE_H
#define AUTOFIRE_H

#ifndef NO_AUTOFIRE

#define MAX_PLAYER 4
#define MAX_AUTOFIRE_BUTTONS 6

enum { P_1 = 0, P_2, P_3, P_4, };
enum { F_1 = 0, F_2, F_3, F_4, F_5, F_6, };

void __cdecl initAutofire();
void __cdecl doAutofire();
int setAutofire(const int& player, const int& fire, bool state);
bool getAutofire(const int& player, const int& fire);

#endif

#endif
