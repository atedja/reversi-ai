#include "lib.h"
#include "handler.h"
#include "common.h"


const double POS_INFINITY = 10000000.0;
const double NEG_INFINITY = -10000000.0;


// PrintBoard()
// Prints a board
void PrintBoard( const Reversi::board_type& board )
{
   using namespace std;
   cout << " |1 2 3 4 5 6 7 8 \n" << "==================\n";
   for( int i=11; i<89; i+=10 )
   {
      cout << i/10 << "|";
      cout << board[i] << " " << board[i+1] << " " << board[i+2] << " " << board[i+3] << " " << board[i+4] << " " << board[i+5] << " " << board[i+6] << " " << board[i+7] << endl;
   }
}


// TranslateBoardtoNN()
// Translates board 'board' for player 'player' to input 'nn_out' recognizable by the NN
void TranslateBoardtoNN( const Reversi::board_type& board, Reversi::value_type player, NeuralNetwork::nodes_type& nn_out )
{
   Reversi::value_type opp_pl = ( player == Reversi::BLACK ) ? Reversi::WHITE : Reversi::BLACK;
   nn_out.resize(1152);
   int j = 0;
   for( int i=11; i<89; i++ )
   {
      int idiv = i/10;
      int imod = i%10;

      if ( imod > 8 || imod < 1 ) continue;

      nn_out[j+0] = (idiv == 1) ? 1.0 : 0.0;
      nn_out[j+1] = (idiv == 2) ? 1.0 : 0.0;
      nn_out[j+2] = (idiv == 3) ? 1.0 : 0.0;
      nn_out[j+3] = (idiv == 4) ? 1.0 : 0.0;
      nn_out[j+4] = (idiv == 5) ? 1.0 : 0.0;
      nn_out[j+5] = (idiv == 6) ? 1.0 : 0.0;
      nn_out[j+6] = (idiv == 7) ? 1.0 : 0.0;
      nn_out[j+7] = (idiv == 8) ? 1.0 : 0.0;

      nn_out[j+8]  = (imod == 1) ? 1.0 : 0.0;
      nn_out[j+9]  = (imod == 2) ? 1.0 : 0.0;
      nn_out[j+10] = (imod == 3) ? 1.0 : 0.0;
      nn_out[j+11] = (imod == 4) ? 1.0 : 0.0;
      nn_out[j+12] = (imod == 5) ? 1.0 : 0.0;
      nn_out[j+13] = (imod == 6) ? 1.0 : 0.0;
      nn_out[j+14] = (imod == 7) ? 1.0 : 0.0;
      nn_out[j+15] = (imod == 8) ? 1.0 : 0.0;

      nn_out[j+16] = (board[i] == player) ? 1.0f : 0.0f;
      nn_out[j+17] = (board[i] == opp_pl) ? 1.0f : 0.0f;

      j+=18;

      //nn_out[j+2] = (board[i+1] == player) ? 1.0f : 0.0f;
      //nn_out[j+3] = (board[i+1] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+4] = (board[i+2] == player) ? 1.0f : 0.0f;
      //nn_out[j+5] = (board[i+2] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+6] = (board[i+3] == player) ? 1.0f : 0.0f;
      //nn_out[j+7] = (board[i+3] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+8] = (board[i+4] == player) ? 1.0f : 0.0f;
      //nn_out[j+9] = (board[i+4] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+10] = (board[i+5] == player) ? 1.0f : 0.0f;
      //nn_out[j+11] = (board[i+5] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+12] = (board[i+6] == player) ? 1.0f : 0.0f;
      //nn_out[j+13] = (board[i+6] == opp_pl) ? 1.0f : 0.0f;

      //nn_out[j+14] = (board[i+7] == player) ? 1.0f : 0.0f;
      //nn_out[j+15] = (board[i+7] == opp_pl) ? 1.0f : 0.0f;

      //j+=16;
   }
}


// ----------------- HUMAN -----------------
HumanHandler::HumanHandler( bool v ) : _verbose(v)
{
}

void HumanHandler::SetColor( Reversi::value_type col )
{
   _color = col;
   if ( _color == Reversi::WHITE ) _colorstr = "WHITE";
   else _colorstr = "BLACK";
}

Reversi::index_type HumanHandler::operator() (const Reversi::board_type& board, Reversi::move_list& moves)
{
   using namespace std;
   PrintBoard( board );
   if ( _verbose )
   {
      Reversi::move_list::iterator it = moves.begin();
      cout << "Available moves: ";
      while ( it != moves.end() )
      {
         cout << *it << " ";
         ++it;
      }
      cout << "\n";
   }
   Reversi::index_type m;
   cout << _colorstr << " TURN. Your move: ";
   cin >> m;
   cout << "\n";

   return m;
}


// ----------------- NEURAL NETWORK COMPUTER -----------------
NNComputer::NNComputer( bool v ) : _verbose(v), _depth(1)
{
}

void NNComputer::SetDepth( int d )
{
   _depth = d;
}

void NNComputer::SetNN( Population::Individual* nind )
{
	_ind = nind;
}

void NNComputer::SetColor( Reversi::value_type col )
{
   _color = col;
   if ( _color == Reversi::WHITE ) _colorstr = "WHITE";
   else _colorstr = "BLACK";
   _opp_color = ( _color == Reversi::WHITE ) ? Reversi::BLACK : Reversi::WHITE;
}


// BestMove()
// Top level MAX, slightly different than the other MAXs because it returns the best move
// Returns the best move
Reversi::index_type NNComputer::BestMove( const Reversi::board_type& board, Reversi::move_list& moves, int depth )
{
   // find the best move
   NeuralNetwork::value_type alpha = NEG_INFINITY;
   NeuralNetwork::value_type beta = POS_INFINITY;
   Reversi::board_type bd;
   Reversi::index_type best_move = 0, move = 0;
   NeuralNetwork::value_type res;
   Reversi::move_list::iterator it = moves.begin();
   while( it != moves.end() )
   {
      move = *it;
      bd = board;
      Reversi::Perform( bd, _color, move );
      res = MinMove( bd, alpha, beta, depth-1 );
      if ( res > alpha )
      {
         best_move = move;
         alpha = res;
      }

      ++it;
   }
   return best_move;
}


// MinMove()
// Min Tree.
// Returns the value of the worst move made by the opponent
NeuralNetwork::value_type NNComputer::MinMove( const Reversi::board_type& board, NeuralNetwork::value_type alpha, NeuralNetwork::value_type beta, int depth )
{
   // end of search tree
   if ( depth == 0 )
   {
      NeuralNetwork::nodes_type input;
      TranslateBoardtoNN( board, _color, input );
      _ind->nn.Input( input );
      _ind->nn.FeedForward();
      return _ind->nn.GetOutput();
   }

   // or no more move available for the opponent, this becomes a MAX
   Reversi::move_list moves;
   if ( !Reversi::MoveAvailable(board, _opp_color, moves) )
   {
      return MaxMove( board, alpha, beta, depth-1);
   }

   // for each move available..
   Reversi::board_type bd;
   Reversi::index_type best_move = 0, move = 0;
   NeuralNetwork::value_type res, best_res = POS_INFINITY;
   Reversi::move_list::iterator it = moves.begin();
   while( it != moves.end() )
   {
      move = *it;
      bd = board;
      Reversi::Perform( bd, _opp_color, move );
      res = MaxMove( bd, alpha, beta, depth-1 );
      if ( res < best_res )
      {
         best_res = res;
         best_move = move;
         if ( best_res < beta ) beta = best_res;
      }

      if ( beta < alpha ) return beta;

      ++it;
   }
   return best_res;
}

// MaxMove()
// Max Tree.
// Returns the value of the best move made by this player
NeuralNetwork::value_type NNComputer::MaxMove( const Reversi::board_type& board, NeuralNetwork::value_type alpha, NeuralNetwork::value_type beta, int depth )
{
   // end of search tree
   if ( depth == 0 )
   {
      NeuralNetwork::nodes_type input;
      TranslateBoardtoNN( board, _color, input );
      _ind->nn.Input( input );
      _ind->nn.FeedForward();
      return _ind->nn.GetOutput();
   }

   // or no more move available for this player, this becomes a MIN
   Reversi::move_list moves;
   if ( !Reversi::MoveAvailable(board, _color, moves) )
   {
      return MinMove( board, alpha, beta, depth-1);
   }

   // for each move available..
   Reversi::board_type bd;
   Reversi::index_type best_move = 0, move = 0;
   NeuralNetwork::value_type res, best_res = NEG_INFINITY;
   Reversi::move_list::iterator it = moves.begin();
   while( it != moves.end() )
   {
      move = *it;
      bd = board;
      Reversi::Perform( bd, _color, move );
      res = MinMove( bd, alpha, beta, depth-1 );
      if ( res > best_res )
      {
         best_res = res;
         best_move = move;
         if ( best_res > alpha ) alpha = best_res;
      }

      if ( beta < alpha ) return alpha;

      ++it;
   }
   return best_res;
}

Reversi::index_type NNComputer::operator() (const Reversi::board_type& board, Reversi::move_list& moves)
{
   using namespace std;
   if ( _verbose )
   {
      PrintBoard( board );
      cout << "Available moves: ";
      Reversi::move_list::iterator it = moves.begin();
      while ( it != moves.end() )
      {
         cout << *it << " ";
         ++it;
      }
      cout << "\n";
      cout << "Computer thinking..."; cout.flush();
   }

   Reversi::index_type best_move = 0;
   if ( _depth == 1 )
   {
      // find the best move
      NeuralNetwork::nodes_type input;
      Reversi::board_type bd;
      Reversi::index_type move = 0;
      NeuralNetwork::value_type best_res = NEG_INFINITY, res = NEG_INFINITY;
      Reversi::move_list::iterator it = moves.begin();
      while( it != moves.end() )
      {
         move = *it;
         bd = board;
         Reversi::Perform( bd, _color, move );
         TranslateBoardtoNN( bd, _color, input );
         _ind->nn.Input( input );
         _ind->nn.FeedForward();
         res = _ind->nn.GetOutput();
         if ( res > best_res )
         {
            best_move = move;
            best_res = res;
         }

         ++it;
      }
   }
   else
   {
      best_move = BestMove( board, moves, _depth );
   }

   if ( _verbose )
      cout << "\rCOMPUTER TURN [" << _colorstr << "]. Computer move: " << best_move << "\n\n";

   return best_move;
}


// ----------------- RANDOM COMPUTER -----------------
RandomComputer::RandomComputer( bool v ) : _verbose(v)
{
}

void RandomComputer::SetColor( Reversi::value_type col )
{
   _color = col;
   if ( _color == Reversi::WHITE ) _colorstr = "WHITE";
   else _colorstr = "BLACK";
   _opp_color = ( _color == Reversi::WHITE ) ? Reversi::BLACK : Reversi::WHITE;
}

Reversi::index_type RandomComputer::operator()( const Reversi::board_type& board, Reversi::move_list& moves )
{
   using namespace std;
   if ( _verbose )
   {
      PrintBoard( board );
      cout << "Available moves: ";
      Reversi::move_list::iterator it = moves.begin();
      while ( it != moves.end() )
      {
         cout << *it << " ";
         ++it;
      }
      cout << "\n";
   }

   // pick a random move
   std::vector<Reversi::index_type> movesv(moves.begin(), moves.end());
   int m = (int) randf(0.0,double(movesv.size()));
   Reversi::index_type best_move = movesv[m];

   if ( _verbose )
      cout << "COMPUTER TURN [" << _colorstr << "]. Computer move: " << best_move << "\n\n";
   
   return best_move;
}
