#ifndef PLAYER_HANDLER_H_
#define PLAYER_HANDLER_H_

#include "reversi.h"
#include "nn.h"
#include "population.h"


void PrintBoard( const Reversi::board_type& board );

class HumanHandler : public Reversi::PlayerHandler
{
   Reversi::value_type  _color;
   std::string          _colorstr;
   bool                 _verbose;
   
public:
   HumanHandler( bool v );
   void SetColor( Reversi::value_type col );
   Reversi::index_type operator()( const Reversi::board_type& board, Reversi::move_list& moves );
};


class NNComputer : public Reversi::PlayerHandler
{
   Reversi::value_type     _color;
   Reversi::value_type     _opp_color;
   std::string             _colorstr;
   Population::Individual* _ind;
   bool                    _verbose;
   int                     _depth;

private:
   Reversi::index_type BestMove( const Reversi::board_type& board, Reversi::move_list& moves, int depth );
   NeuralNetwork::value_type MinMove( const Reversi::board_type& board, NeuralNetwork::value_type alpha, NeuralNetwork::value_type beta, int depth );
   NeuralNetwork::value_type MaxMove( const Reversi::board_type& board, NeuralNetwork::value_type alpha, NeuralNetwork::value_type beta, int depth );

public:
   NNComputer( bool v );
   void SetDepth( int d );
   void SetNN( Population::Individual* nind );
   void SetColor( Reversi::value_type col );
   Reversi::index_type operator()( const Reversi::board_type& board, Reversi::move_list& moves );
};


class RandomComputer : public Reversi::PlayerHandler
{
   Reversi::value_type     _color;
   Reversi::value_type     _opp_color;
   std::string             _colorstr;
   bool                    _verbose;

public:
   RandomComputer( bool v );
   void SetColor( Reversi::value_type col );
   Reversi::index_type operator()( const Reversi::board_type& board, Reversi::move_list& moves );
};


#endif
