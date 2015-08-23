#ifndef __RANDOMGENERATOR_H
#define __RANDOMGENERATOR_H

#include <stdlib.h>
#include <time.h>

template<int _Precise>	//���е�
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
	inline bool operator [](float rate)	// rateȮ���� true ����
	{
		return *this < rate;
	}
};

extern RandomGenerator<1000>	g_rnd;	// 0<=r<1�� ���� �����(���е� 1/1000 : 72.32%�� 72.33%�� ���� ����) 1000���� ������ ������� �� ���� �ڼ���, �Ϲ����� �뵵�� ���
extern RandomGenerator<1000000>	g_rndM;	// 0<=r<1�� ���� �����(���е� 1/Million) - 1�鸸���� ������ ������� �� ���� �ڼ���(�̶� �ణ�� ������), ������ �巴�ÿ��� ���

#endif  __RANDOMGENERATOR_H
