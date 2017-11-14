#ifndef ALNITE_NN_H_
#define ALNITE_NN_H_

class NeuralNetwork
{
public:
   typedef double                   value_type;
   typedef std::vector<value_type>  nodes_type;
   typedef std::vector<int>         layer_info_type;

   struct _link
   {
      int src, dest;
      value_type weight;
   };

   typedef std::vector<_link>       weight_type;

private:
   nodes_type        _nodes;        // all nodes
   weight_type       _weights;      // all weights
   layer_info_type   _layer_info;   // number of nodes in each layer(index)

   int               _layer_count;
   int               _nodes_count;
   int               _weight_count;

public:
   NeuralNetwork();

   void Create( const char* );
   void Create( const char*, const weight_type& );
   void ReplaceWeight( const weight_type& );
   void Input( nodes_type& );
   void FeedForward();

   const nodes_type& GetNodes() const;
   const weight_type& GetWeights() const;
   const layer_info_type& GetLayerInfo() const;

   value_type GetOutput() const;
   int LayerCount() const;
   int HiddenLayerCount() const;
   int InputNodesCount() const;
   int OutputNodesCount() const;
   int NodesCount() const;
   int WeightCount() const;
};


#endif
