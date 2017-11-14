#include "lib.h"
#include "reversi.h"

// Reversi Engine
// Version 1.1
// Copyright(C) Albert Tedja. All Rights Reserved.
// April 4th, 2006

// Reversi board numbering system
// +----+----+----+----+----+----+----+----+----+----+
// | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 |
// +----+----+----+----+----+----+----+----+----+----+
// | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 |
// +----+----+----+----+----+----+----+----+----+----+
// | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 |
// +----+----+----+----+----+----+----+----+----+----+
// | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |
// +----+----+----+----+----+----+----+----+----+----+
// | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 |
// +----+----+----+----+----+----+----+----+----+----+
// | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 |
// +----+----+----+----+----+----+----+----+----+----+
// | 60 | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 | 69 |
// +----+----+----+----+----+----+----+----+----+----+
// | 70 | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79 |
// +----+----+----+----+----+----+----+----+----+----+
// | 80 | 81 | 82 | 83 | 84 | 85 | 86 | 87 | 88 | 89 |
// +----+----+----+----+----+----+----+----+----+----+
// | 90 | 91 | 92 | 93 | 94 | 95 | 96 | 97 | 98 | 99 |
// +----+----+----+----+----+----+----+----+----+----+
// 
// using only the middle 8x8:
//
// +----+----+----+----+----+----+----+----+
// | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 |
// +----+----+----+----+----+----+----+----+
// | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 |
// +----+----+----+----+----+----+----+----+
// | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 |
// +----+----+----+----+----+----+----+----+
// | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 |
// +----+----+----+----+----+----+----+----+
// | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 |
// +----+----+----+----+----+----+----+----+
// | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 |
// +----+----+----+----+----+----+----+----+
// | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 |
// +----+----+----+----+----+----+----+----+
// | 81 | 82 | 83 | 84 | 85 | 86 | 87 | 88 |
// +----+----+----+----+----+----+----+----+


const Reversi::value_type Reversi::EMPTY = '.';
const Reversi::value_type Reversi::WHITE = 'w';
const Reversi::value_type Reversi::BLACK = 'b';
const int BOARD_SIZE = 100;

Reversi::Reversi() : _reversi(BOARD_SIZE)
{
   _endgame_func = 0;
   _startgame_func = 0;
}


// Start()
// Starts the game.
void Reversi::Start( Reversi::PlayerHandler& w, Reversi::PlayerHandler& b )
{
   // initialize board
   for( index_type i=0; i<BOARD_SIZE; ++i )
      _reversi[i] = EMPTY;
   _reversi[44] = _reversi[55] = WHITE;
   _reversi[45] = _reversi[54] = BLACK;

   // set callback functions
   PlayerHandler& white_func = w;
   PlayerHandler& black_func = b;

   // trigger event
   _running = true;
   if ( _startgame_func ) (*_startgame_func)(_reversi);

   // run game
   bool black_avail = true;
   bool white_avail = true;
   move_list black_moves, white_moves;
   index_type move;
   while ( _running )
   {
      if ( (black_avail = MoveAvailable(_reversi,BLACK,black_moves)) )
      {
         do { move = black_func(_reversi,black_moves); }
         while ( !Perform(_reversi,BLACK,move) );
      }

      if ( (white_avail = MoveAvailable(_reversi,WHITE,white_moves)) )
      {
         do { move = white_func(_reversi,white_moves); }
         while ( !Perform(_reversi,WHITE,move) );
      }

      if ( !(black_avail || white_avail) ) { End(); break; }
   }
}


// End()
// Ends the game.
void Reversi::End()
{
   _running = false;
   if ( _endgame_func ) (*_endgame_func)(_reversi);
}


// _Switch()
// Performs switching pieces for player 'player' at 'start' in the direction of 'dy'.
bool Reversi::_Switch( Reversi::board_type& board, Reversi::value_type player, Reversi::index_type start, int dy )
{
   value_type opp_pl = ( player == BLACK ) ? WHITE : BLACK;
   bool found = false;
   index_type j = int(start + dy);
   value_type v = board[j];
   while ( v == opp_pl && !found )
   {
      j += dy;
      v = board[j];
      if ( v == player )
      {
         found = true;
         while( j != start )
         { board[j] = player; j -= dy; }
         break;
      }
   }
   return found;
}


// _Finds()
// Finds if a move is available for player 'player' at 'start' in the direction of 'dy'.
// Returns the position of the available move
Reversi::index_type Reversi::_Finds( const Reversi::board_type& board, Reversi::value_type player, Reversi::index_type start, int dy )
{
   value_type opp_pl = ( player == BLACK ) ? WHITE : BLACK;
   bool found = false;
   index_type j = int(start + dy);
   value_type v = board[j];
   while ( v == opp_pl && !found )
   {
      j += dy;
      v = board[j];
      if ( v == EMPTY && !(j<11 || j>88 || j%10==0 || j%10==9) ) { found = true; break; }
   }
   if ( found ) return j;
   return 0;
}


// SetStartHandler(), SetEndHandler()
// Set game event handlers
void Reversi::SetStartHandler( Reversi::GameEventHandler* h )
{
   _startgame_func = h;
}

void Reversi::SetEndHandler( Reversi::GameEventHandler* h )
{
   _endgame_func = h;
}


// CountPieces()
// Counts the pieces of white and black players to to 'w' and 'b', respectively.
void Reversi::CountPieces( int& w, int& b ) const
{
   w = b = 0;
   value_type v;
   for( index_type i=11; i<89; ++i )
   {
      v = _reversi[i];
      if ( v == BLACK ) b++;
      else if ( v == WHITE ) w++;
   }
}


// Perform()
// Checks for valid moves at 'i' for player 'player' and switches opponent pieces if found.
// Returns true if a move can be performed, false otherwise
bool Reversi::Perform( Reversi::board_type& board, Reversi::value_type player, Reversi::index_type i )
{
   // check for out of bounds and occupied spot
   if ( i < 11 || i > 88 || i%10==0 || i%10==9 ) return false;
   if ( board[i] != EMPTY ) return false;

   // check around
   bool result[8] = {false};
   result[0] = _Switch( board, player, i, -10 );       // top
   result[1] = _Switch( board, player, i, +10 );       // bottom
   result[2] = _Switch( board, player, i, -1 );        // left
   result[3] = _Switch( board, player, i, +1 );        // right
   result[4] = _Switch( board, player, i, -11 );       // top-left
   result[5] = _Switch( board, player, i, +11 );       // bottom-right
   result[6] = _Switch( board, player, i, -9 );        // top-right
   result[7] = _Switch( board, player, i, +9 );        // bottom-left
   if ( result[0] || result[1] || result[2] || result[3] || result[4] || result[5] || result[6] || result[7] )
   { board[i] = player; return true; }

   return false;
}


// MoveAvailable()
// Finds if a move is available for player 'player'.
// Returns all available moves.
bool Reversi::MoveAvailable( const Reversi::board_type& board, Reversi::value_type player, Reversi::move_list& moves )
{
   moves.clear();
   index_type res;
   for ( index_type i=11; i<89; ++i )
   {
      if ( board[i] == player )
      {
         res = _Finds(board, player, i, -10); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i, +10); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i,  -1); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i,  +1); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i, -11); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i, +11); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i,  -9); if ( res > 0 ) moves.insert(res);
         res = _Finds(board, player, i,  +9); if ( res > 0 ) moves.insert(res);
      }
   }
   if ( moves.size() ) return true;
   return false;
}


// GetBoard()
// Returns the content of the whole 10x10 board.
Reversi::board_type Reversi::GetBoard() const
{
   return _reversi;
}
