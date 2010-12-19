#include "FieldOperator.h"

OperatorContainer::OperatorContainer(StatesClassification &System, Hamiltonian &H, output_handle &OUT, int bit) : 
    System(System), H(H), OUT(OUT), bit(bit)
{
    size=0;
}

CreationOperator::CreationOperator(StatesClassification &System, Hamiltonian &H, output_handle &OUT, int bit) : 
    OperatorContainer(System,H,OUT,bit)
{}

AnnihilationOperator::AnnihilationOperator(StatesClassification &System, Hamiltonian &H, output_handle &OUT, int bit) : 
    OperatorContainer(System,H,OUT,bit)
{}

std::list<BlockMapping>& OperatorContainer::getNonTrivialIndices()
{
    return LeftRightIndices;
};

FieldOperatorPart& OperatorContainer::getPartFromRightIndex(BlockNumber in)
{
  return *Data[mapPartsFromRight[in]];
}

FieldOperatorPart& OperatorContainer::getPartFromRightIndex(QuantumNumbers in)
{
  return *Data[mapPartsFromRight[System.getBlockNumber(in)]];
}

FieldOperatorPart& OperatorContainer::getPartFromLeftIndex(BlockNumber in)
{
  return *Data[mapPartsFromLeft[in]];
}

FieldOperatorPart& OperatorContainer::getPartFromLeftIndex(QuantumNumbers in)
{
  return *Data[mapPartsFromLeft[System.getBlockNumber(in)]];
}

void OperatorContainer::print_to_screen()
{
  for (unsigned int b_in=0;b_in<(*this).size;b_in++)
  {
        Data[b_in]->print_to_screen();
  };
}

void OperatorContainer::compute()
{
  for (unsigned int b_in=0;b_in<(*this).size;b_in++)
  {
    cout << (int) ((1.0*b_in/(*this).size) * 100 ) << "  " << flush;
        Data[b_in]->compute();
  };
  cout << endl;
}

void OperatorContainer::dump()
{
  for (unsigned int b_in=0;b_in<(*this).size;b_in++)
  {
        Data[b_in]->dump();
  };
}

unsigned short OperatorContainer::getBit() const
{ 
    return bit;
}

void CreationOperator::prepare()
{
  for (BlockNumber RightIndex=0;RightIndex<System.NumberOfBlocks();RightIndex++)
    {
      BlockNumber LeftIndex = mapsTo(RightIndex);
      if (LeftIndex.isCorrect()) 
      {
         FieldOperatorPart *Part = new CreationOperatorPart(bit,System,H.part(RightIndex),H.part(LeftIndex),OUT);
         Data.push_back(Part);
         cout << "Entering creation operator part " << System.getBlockInfo(RightIndex) << "->" << System.getBlockInfo(LeftIndex) << endl; 
         mapPartsFromRight[RightIndex]=size;
         mapPartsFromLeft[LeftIndex]=size;
         LeftRightIndices.push_back(BlockMapping(LeftIndex,RightIndex));
	 mapRightToLeftIndex[RightIndex]=LeftIndex;
	 mapLeftToRightIndex[LeftIndex]=RightIndex;
         size++;
      }    
    }
}

void AnnihilationOperator::prepare()
{
  for (BlockNumber RightIndex=0;RightIndex<System.NumberOfBlocks();RightIndex++)
    {
      BlockNumber LeftIndex = mapsTo(RightIndex);
      if (LeftIndex.isCorrect()) 
      {
         FieldOperatorPart *Part = new AnnihilationOperatorPart(bit,System,H.part(RightIndex),H.part(LeftIndex),OUT);
         Data.push_back(Part);
         cout << "Entering annihilation operator part " << System.getBlockInfo(RightIndex) << "->" << System.getBlockInfo(LeftIndex) << endl; 
         mapPartsFromRight[RightIndex]=size;
         mapPartsFromLeft[LeftIndex]=size;
	 mapRightToLeftIndex[RightIndex]=LeftIndex;
	 mapLeftToRightIndex[LeftIndex]=RightIndex;
         LeftRightIndices.push_back(BlockMapping(LeftIndex,RightIndex));
         size++;
      }    
    }
}

BlockNumber OperatorContainer::getRightIndex(BlockNumber LeftIndex)
{
	return mapLeftToRightIndex.count(LeftIndex)?mapLeftToRightIndex[LeftIndex]:ERROR_BLOCK_NUMBER;
};

BlockNumber OperatorContainer::getLeftIndex(BlockNumber RightIndex)
{
	return mapRightToLeftIndex.count(RightIndex)?mapRightToLeftIndex[RightIndex]:ERROR_BLOCK_NUMBER;
};

QuantumNumbers CreationOperator::mapsTo(QuantumNumbers in) // Require explicit knowledge of QuantumNumbers structure - Not very good
{
  int lz, spin;
  System.getSiteInfo(bit,lz,spin);
  QuantumNumbers q_out;
  if (spin == 1) 
    q_out = QuantumNumbers(in.Lz + lz,in.N_up+1,in.N_down);
  else 
    q_out = QuantumNumbers(in.Lz + lz,in.N_up,in.N_down+1);
  return System.checkQuantumNumbers(q_out)?q_out:ERROR_QUANTUM_NUMBERS;
}

BlockNumber CreationOperator::mapsTo(BlockNumber RightIndex)
{
  QuantumNumbers q_right = System.getBlockInfo(RightIndex);	
  QuantumNumbers q_left = (*this).mapsTo(q_right);
  BlockNumber out = System.getBlockNumber(q_left);
  return out;
}

QuantumNumbers AnnihilationOperator::mapsTo(QuantumNumbers in) // Require explicit knowledge of QuantumNumbers structure - Not very good
{
  int lz, spin;
  System.getSiteInfo(bit,lz,spin);
  QuantumNumbers q_out;
  if (spin == 1) 
    q_out = QuantumNumbers(in.Lz - lz,in.N_up-1,in.N_down);
  else 
    q_out = QuantumNumbers(in.Lz - lz,in.N_up,in.N_down-1);
  return System.checkQuantumNumbers(q_out)?q_out:ERROR_QUANTUM_NUMBERS;
}

BlockNumber AnnihilationOperator::mapsTo(BlockNumber in)
{
  QuantumNumbers q_in = System.getBlockInfo(in);	
  QuantumNumbers q_out = (*this).mapsTo(q_in);
  BlockNumber out = System.getBlockNumber(q_out);
  return out;
}
