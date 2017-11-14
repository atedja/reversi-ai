#include "lib.h"
#include "common.h"
#include "reversi.h"
#include "nn.h"
#include "handler.h"
#include "population.h"

// Constants
const int PLAYER_HUMAN     = 1;
const int PLAYER_COMPUTER  = 2;

const char* FILE_CURRENT_GEN  = "current.pop";
const char* FILE_PREV_GEN     = "prev.pop";
const char* FILE_NN_CONF      = "nn.conf";

const char* CMD_PLAY =  "-p";
const char* CMD_TRAINNN = "-en";
const char* CMD_TRAINRM = "-er";

// Functions
void Play( bool verbose, int black, int white, Population::Individual* cwp, Population::Individual* cbp );
void DisplayOptions();


// Global Variables
int white_player = PLAYER_HUMAN;
int black_player = PLAYER_COMPUTER;

bool nn_from_file = true;

Population curr_gen;


// Play()
// Plays a normal game
void Play( bool verbose, int white, int black, Population::Individual* cwp, Population::Individual* cbp )
{
   white_player = white;
   black_player = black;
   
   HumanHandler human( verbose );

   NNComputer computer_nn_white( verbose );
   computer_nn_white.SetColor( Reversi::WHITE );
   computer_nn_white.SetDepth( 7 );
   computer_nn_white.SetNN( cwp );

   NNComputer computer_nn_black( verbose );
   computer_nn_black.SetColor( Reversi::BLACK );
   computer_nn_black.SetDepth( 7 );
   computer_nn_black.SetNN( cbp );
   
   Reversi::PlayerHandler* wp = 0;
   Reversi::PlayerHandler* bp = 0;
   if ( white_player == PLAYER_HUMAN ) { human.SetColor(Reversi::WHITE); wp = &human; }
   else wp = &computer_nn_white;

   if ( black_player == PLAYER_HUMAN ) { human.SetColor(Reversi::BLACK); bp = &human; }
   else bp = &computer_nn_black;

   Reversi game;
   game.Start( *wp, *bp );
   int w, b;
   PrintBoard( game.GetBoard() );
   game.CountPieces( w, b );
   std::cout << "WHITE: " << w << "\tBLACK: " << b << std::endl;
}


// DisplayOptions()
// Displays command-line options
void DisplayOptions()
{
   using namespace std;
   cout << "Usage: rnn [options]\n";
   cout << "Options:\n";
   cout << "  -en X        Trains neural networks for X generations since the last train.\n";
   cout << "               Evolved against neural networks.\n";
   cout << "               Example: -en 10 (train for 10 generations)\n";
   cout << "  -er X        Trains neural networks for X generations since the last train.\n";
   cout << "               Against a random mover.\n";
   cout << "               Example: -er 10 (train for 10 generations)\n";
   cout << "  -p BW        Plays a single game. B and W specifies black and white players,\n";
   cout << "               respectively. Specify 'h' for human and 'c' for computer player.\n";
   cout << "               Example: -p ch (black is computer, white is human)\n\n";
}


int main( int argc, char** argv )
{
   using namespace std;
   if ( argc == 1 )
   {
      DisplayOptions();
      return 0;
   }

   // seed number
   reseed();
   
   // read commands
   int cmdi = 1;
   string cmdstr = string(argv[cmdi]);
   if ( cmdstr == CMD_PLAY )  // Plays a single game
   {
      if ( argc < 3 )
      {
         // default settings
         cout << "Specify players.\n";
         return 0;
      }
      else
      {
         string opt = string(argv[cmdi+1]);
         if ( opt == "cc" )
         {
            curr_gen.Load( FILE_CURRENT_GEN );
            int bi, wi;
            cout << "Select computer player for BLACK (0-" << curr_gen.GetSize()-1 << "): ";
            cin >> bi;
            cout << "Select computer player for WHITE (0-" << curr_gen.GetSize()-1 << "): ";
            cin >> wi;
            Play( true, PLAYER_COMPUTER, PLAYER_COMPUTER, &curr_gen._population[wi], &curr_gen._population[bi] );
         }
         else if ( opt == "hc" )
         {
            curr_gen.Load( FILE_CURRENT_GEN );
            int wi;
            cout << "Select computer player for WHITE (0-" << curr_gen.GetSize()-1 << "): ";
            cin >> wi;
            Play( true, PLAYER_COMPUTER, PLAYER_HUMAN, &curr_gen._population[wi], 0 );
         }
         else if ( opt == "ch" )
         {
            curr_gen.Load( FILE_CURRENT_GEN );
            int bi;
            cout << "Select computer player for BLACK (0-" << curr_gen.GetSize()-1 << "): ";
            cin >> bi;
            Play( true, PLAYER_HUMAN, PLAYER_COMPUTER, 0, &curr_gen._population[bi] );
         }
         else if ( opt == "hh" )
         {
            Play( true, PLAYER_HUMAN, PLAYER_HUMAN, 0, 0 );
         }
         else
         {
            cout << "Invalid command: '" << cmdstr << " " << opt << "'" << endl;
            return 0;
         }
      }
      
      // if this generation is not from a file, save it
      if ( !nn_from_file ) curr_gen.Save( FILE_CURRENT_GEN );
   }
   else if ( cmdstr == CMD_TRAINNN )
   {
      if ( argc < 3 )
      {
         cout << "Specify #generations to train." << endl;
      }
      else
      {
         string opt = string(argv[cmdi+1]);
         stringstream ss(opt); int gen; ss >> gen;
         curr_gen.Load( FILE_CURRENT_GEN );
         curr_gen.EvolveNN( gen );
         curr_gen.Save( FILE_CURRENT_GEN );
      }
   }
   else if ( cmdstr == CMD_TRAINRM )
   {
      if ( argc < 3 )
      {
         cout << "Specify #generations to train." << endl;
      }
      else
      {
         string opt = string(argv[cmdi+1]);
         stringstream ss(opt); int gen; ss >> gen;
         curr_gen.Load( FILE_CURRENT_GEN );
         curr_gen.EvolveRM( gen );
         curr_gen.Save( FILE_CURRENT_GEN );
      }
   }
   else
   {
      cout << "Invalid command: '" << cmdstr << "'" << endl;
      return 0;
   }

   return 0;
}
