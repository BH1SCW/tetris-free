#ifndef __FRAME_H
#define __FRAME_H

extern RussiaBlock * russia_block;

void clock_frame(Mtime time);
void game_start(RussiaBlock * rb);
void recv_player_data(RussiaBlock * rb);
void send_player_data(RussiaBlock * rb);

#endif
