#ifndef EV_RAS_H
#define EV_RAS_H

#include <iostream>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32

#pragma warning (disable : 4334)

typedef __int64 _INT64_;
typedef unsigned __int64 _UINT64_;

#else

typedef long long _INT64_;
typedef unsigned long long	_UINT64_;

#endif

using namespace std;//RSA�㷨�������

struct  RSA_PARAM
{
	_UINT64_    p, q;   //������������������ܽ�������
	_UINT64_    f;      //f=(p-1)*(q-1)����������ܽ�������
	_UINT64_    n, e;   //���ף�n=p*q��gcd(e,f)=1 
	_UINT64_    d;      //˽�ף�e*d=1 (mod f)��gcd(n,d)=1
	_UINT64_    s;      //�鳤������2^s<=n������s����log2(n)

	RSA_PARAM(){};

	RSA_PARAM(_UINT64_ p64, _UINT64_ q64, _UINT64_ f64, _UINT64_ n64, _UINT64_ e64, _UINT64_ d64, _UINT64_ s64)
	{
		p = p64;
		q = q64;
		f = f64;
		n = n64;
		e = e64;
		d = d64;
		s = s64;
	}
} ;//С������

class  RandNumber
{
private:
	_UINT64_ randSeed;
public:
	RandNumber(_UINT64_ s = 0);
	_UINT64_ Random(_UINT64_ n);
};

// ģ�����㣬����ֵ x = a*b mod n
inline _UINT64_ MulMod(_UINT64_ a, _UINT64_ b, _UINT64_ n)
{
	return a * b % n;
}

// ģ�����㣬����ֵ x=base^pow mod n
_UINT64_ PowMod(_UINT64_ &base, _UINT64_ &pow, _UINT64_ &n);

// Rabin-Miller�������ԣ�ͨ�����Է���1�����򷵻�0��
// n�Ǵ���������
// ע�⣺ͨ�����Բ���һ������������������ͨ�����Եĸ�����1/4
long RabinMillerKnl(_UINT64_ &n);

// Rabin-Miller�������ԣ�ѭ�����ú���loop��
// ȫ��ͨ������1�����򷵻�0
long RabinMiller(_UINT64_ &n, long loop);

// �������һ��bitsλ(������λ)�����������32λ
_UINT64_ RandomPrime(char bits);

// ŷ����÷������Լ��
_UINT64_ EuclidGcd(_UINT64_ &p, _UINT64_ &q);

// Stein�������Լ��
_UINT64_ SteinGcd(_UINT64_ &p, _UINT64_ &q);

// ��֪a��b����x������a*x =1 (mod b)
// �൱�����a*x-b*y=1����С������
_UINT64_ Euclid(_UINT64_ &a, _UINT64_ &b);

// �������һ��RSA���ܲ���
RSA_PARAM RsaGetParam(void);

#endif