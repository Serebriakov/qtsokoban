/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Sean Harmer <sh@astro.keele.ac.uk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <Q3ptrlist>

#include <windows.h>  //getpid()


#include "randomsequence.h"
#include <stdlib.h> // getenv(), srand(), rand()

const int    RandomSequence::m_nShuffleTableSize = 32;

//////////////////////////////////////////////////////////////////////////////
//	Construction / Destruction
//////////////////////////////////////////////////////////////////////////////

RandomSequence::RandomSequence( long lngSeed1 )
{
  // Seed the generator
  setSeed( lngSeed1 );
	
	
  // Set the size of the shuffle table
  m_ShuffleArray = new long [m_nShuffleTableSize];
}

RandomSequence::~RandomSequence()
{
  delete [] m_ShuffleArray;
}

RandomSequence::RandomSequence(const RandomSequence &a)
{
  // Set the size of the shuffle table
  m_ShuffleArray = new long [m_nShuffleTableSize];
  *this = a;
}

RandomSequence &
RandomSequence::operator=(const RandomSequence &a)
{
  m_lngSeed1 = a.m_lngSeed1;
  m_lngSeed2 = a.m_lngSeed2;
  m_lngShufflePos = a.m_lngShufflePos;
  memcpy(m_ShuffleArray, a.m_ShuffleArray, sizeof(long)*m_nShuffleTableSize);
  return *this;
}


int
getpid(void)
{
	return GetCurrentThreadId();
}


int random()
{
   static bool init = false;
   if (!init)
   {
      unsigned int seed;
      init = true;
   //   int fd = fopen("/dev/urandom", "r");
   //   if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
      {
            // No /dev/urandom... try something else.
            srand(getpid());
            seed = rand()+time(0);
      }
    //  if (fd >= 0) close(fd);
      srand(seed);
   }
   return rand();
}


//////////////////////////////////////////////////////////////////////////////
//	Member Functions
//////////////////////////////////////////////////////////////////////////////

void RandomSequence::setSeed( long lngSeed1 )
{
  // Convert the positive seed number to a negative one so that the Draw()
  // function can intialise itself the first time it is called. We just have
  // to make sure that the seed used != 0 as zero perpetuates itself in a
  // sequence of random numbers.
  if ( lngSeed1 < 0 )
  {
    m_lngSeed1 = -1;
  }
  else if (lngSeed1 == 0)
  {
    m_lngSeed1 = -((random() & ~1)+1);
  }
  else
  {
    m_lngSeed1 = -lngSeed1;
  }
}

static const long sMod1           = 2147483563;
static const long sMod2           = 2147483399;

void RandomSequence::Draw()
{
  static const long sMM1            = sMod1 - 1;
  static const long sA1             = 40014;
  static const long sA2             = 40692;
  static const long sQ1             = 53668;
  static const long sQ2             = 52774;
  static const long sR1             = 12211;
  static const long sR2             = 3791;
  static const long sDiv            = 1 + sMM1 / m_nShuffleTableSize;

  // Long period (>2 * 10^18) random number generator of L'Ecuyer with
  // Bayes-Durham shuffle and added safeguards. Returns a uniform random
  // deviate between 0.0 and 1.0 (exclusive of the endpoint values). Call
  // with a negative number to initialize; thereafter, do not alter idum
  // between successive deviates in a sequence. RNMX should approximate
  // the largest floating point value that is less than 1.
	
  int j; // Index for the shuffle table
  long k;
	
  // Initialise
  if ( m_lngSeed1 <= 0 )
  {	
    m_lngSeed2 = m_lngSeed1;

    // Load the shuffle table after 8 warm-ups
    for ( j = m_nShuffleTableSize + 7; j >= 0; j-- )
    {
      k = m_lngSeed1 / sQ1;
      m_lngSeed1 = sA1 * ( m_lngSeed1 - k*sQ1) - k*sR1;
      if ( m_lngSeed1 < 0 )
      {
        m_lngSeed1 += sMod1;
      }
			
      if ( j < m_nShuffleTableSize )
      {
 	m_ShuffleArray[j] = m_lngSeed1;
      }
    }
		
    m_lngShufflePos = m_ShuffleArray[0];
  }
	
  // Start here when not initializing
	
  // Compute m_lngSeed1 = ( lngIA1*m_lngSeed1 ) % lngIM1 without overflows
  // by Schrage's method
  k = m_lngSeed1 / sQ1;
  m_lngSeed1 = sA1 * ( m_lngSeed1 - k*sQ1 ) - k*sR1;
  if ( m_lngSeed1 < 0 )
  {
    m_lngSeed1 += sMod1;
  }
	
  // Compute m_lngSeed2 = ( lngIA2*m_lngSeed2 ) % lngIM2 without overflows
  // by Schrage's method
  k = m_lngSeed2 / sQ2;
  m_lngSeed2 = sA2 * ( m_lngSeed2 - k*sQ2 ) - k*sR2;
  if ( m_lngSeed2 < 0 )
  {
    m_lngSeed2 += sMod2;
  }
	
  j = m_lngShufflePos / sDiv;
  m_lngShufflePos = m_ShuffleArray[j] - m_lngSeed2;
  m_ShuffleArray[j] = m_lngSeed1;
	
  if ( m_lngShufflePos < 1 )
  {
    m_lngShufflePos += sMM1;
  }
}

void 
RandomSequence::modulate(int i)
{
  m_lngSeed2 -= i;
  if ( m_lngSeed2 < 0 )
  {
    m_lngShufflePos += sMod2;
  }
  Draw();  
  m_lngSeed1 -= i;
  if ( m_lngSeed1 < 0 )
  {
    m_lngSeed1 += sMod1;
  }
  Draw();
}

double
RandomSequence::getDouble()
{
  static const double finalAmp         = 1.0 / double( sMod1 );
  static const double epsilon          = 1.2E-7;
  static const double maxRand          = 1.0 - epsilon;
  double temp;
  Draw();
  // Return a value that is not one of the endpoints
  if ( ( temp = finalAmp * m_lngShufflePos ) > maxRand )
  {
    // We don't want to return 1.0
    return maxRand;
  }
  else
  {
    return temp;
  }
}

unsigned long
RandomSequence::getLong(unsigned long max)
{
  Draw();

  return max ? (((unsigned long) m_lngShufflePos) % max) : 0;  
}

bool
RandomSequence::getBool()
{
  Draw();

  return (((unsigned long) m_lngShufflePos) & 1);  
}

class RandomSequenceList : public Q3GList
{
  friend class RandomSequence;
public:
  RandomSequenceList() : Q3GList() { }
  virtual void deleteItem( Q3PtrCollection::Item ) {}
};

void
RandomSequence::randomize(Q3GList *_list)
{
  RandomSequenceList *list = (RandomSequenceList *)_list;
  RandomSequenceList l;
  while(list->count())
     l.append(list->takeFirst());

  list->append(l.takeFirst()); // Start with 1
  while(l.count())
     list->insertAt(getLong(list->count()+1), l.takeFirst());
}

