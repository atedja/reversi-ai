#ifndef ALNITE_REVERSI_H_
#define ALNITE_REVERSI_H_

// Reversi Engine
// Version 1.2
// Copyright(C) Albert Tedja. All Rights Reserved.
// April 4th, 2006

class Reversi
{
public:
   typedef char value_type;
   typedef std::vector<value_type> board_type;
   typedef board_type::size_type index_type;
   typedef std::set<index_type> move_list;

   static const value_type EMPTY;
   static const value_type WHITE;
   static const value_type BLACK;

   class PlayerHandler
   {
   public:
      virtual Reversi::index_type operator() (const Reversi::board_type&, move_list&) = 0;
   };
   
   class GameEventHandler
   {
   public:
      virtual void operator() (const Reversi::board_type&) = 0;
   };

private:
   board_type  _reversi;
   index_type  _board_size;
   bool        _running;

   GameEventHandler*  _endgame_func;
   GameEventHandler*  _startgame_func;

   static bool _Switch( board_type&, value_type player, index_type start, int dy );
   static index_type _Finds( const board_type&, value_type player, index_type start, int dy );

public:
   Reversi();

   void Start( PlayerHandler& w, PlayerHandler& b );
   void SetStartHandler( GameEventHandler* );
   void SetEndHandler( GameEventHandler* );
   void End();
   void CountPieces( int& w, int& b ) const;
   
   static bool Perform( board_type&, value_type, index_type );
   static bool MoveAvailable( const board_type&, value_type, move_list& );
   
   board_type GetBoard() const;
};


#endif
