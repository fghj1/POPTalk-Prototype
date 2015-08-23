#ifndef __RANDOMGENERATOR_H
#define __RANDOMGENERATOR_H

#include <stdlib.h>
#include <time.h>

template<int _Precise>	//정밀도
class RandomGenerator
{
private :
	float		 brg_Buffer[_Precise];
	int			 brg_Pointer;
	unsigned int brd_Seed;
public :
	RandomGenerator(void)
	{
		srand(brd_Seed = (unsigned int)time(NULL));
		for(int k = 0; k < _Precise; ++k)
			brg_Buffer[k] = (float)k / (float)_Precise;
		rg_Shuffle();
		brg_Pointer = 0;
	}
	~RandomGenerator(void)
	{
	}
	void rg_Shuffle(void)
	{
		for(int k = 0; k < _Precise; ++k)
		{
			int r = (int)((double)rand() * _Precise / (double)(RAND_MAX + 1));
			if(r >= _Precise)
			{
				r = 0;
			}
			float tmp = brg_Buffer[k];
			brg_Buffer[k] = brg_Buffer[r];
			brg_Buffer[r] = tmp;
		}
	}
	inline operator float(void)
	{
		if(brg_Pointer >= _Precise)
		{
			rg_Shuffle();
			brg_Pointer = 0;
		}
		++brg_Pointer;
		return brg_Buffer[brg_Pointer - 1];
	}
	inline float delta(float lower, float upper)
	{
		float rnd = *this;
		rnd *= (upper - lower);
		rnd += lower;
		return rnd;
	}
	inline bool operator [](float rate)	// rate확률로 true 리턴
	{
		return *this < rate;
	}
};

extern RandomGenerator<1000>	g_rnd;	// 0<=r<1인 난수 만들기(정밀도 1/1000 : 72.32%와 72.33%를 구분 못함) 1000개의 난수가 만들어진 후 버퍼 뒤섞음, 일반적인 용도로 사용
extern RandomGenerator<1000000>	g_rndM;	// 0<=r<1인 난수 만들기(정밀도 1/Million) - 1백만개의 난수가 만들어진 후 버퍼 뒤섞음(이때 약간의 딜레이), 아이템 드럽시에만 사용

#endif  __RANDOMGENERATOR_H
