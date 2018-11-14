#include "ippcp.h"
#include "ippcore.h"
#include <stdlib.h>
#include <iostream>
#include <stdio.h>


#include <vector>
#include <iterator>


/////////////////////////////////////////////////
// functions for manipulating BN
////////////////////////////////////////////////////

#if !defined _BIGNUMBER_H_
#define _BIGNUMBER_H_

//#include "ippcp.h"

//#include <iostream>
//#include <vector>
//#include <iterator>

using namespace std;

class BigNumber
{
public:
	BigNumber(Ipp32u value = 0);
	BigNumber(Ipp32s value);
	BigNumber(const IppsBigNumState* pBN);
	BigNumber(const Ipp32u* pData, int length = 1, IppsBigNumSGN sgn = IppsBigNumPOS);
	BigNumber(const BigNumber& bn);
	BigNumber(const char *s);
	virtual ~BigNumber();

	// set value
	void Set(const Ipp32u* pData, int length = 1, IppsBigNumSGN sgn = IppsBigNumPOS);
	// conversion to IppsBigNumState
	friend IppsBigNumState* BN(const BigNumber& bn) { return bn.m_pBN; }
	operator IppsBigNumState* () const { return m_pBN; }
	IppsBigNumState* Ctx() const { return m_pBN; }

	// some useful constatns
	static const BigNumber& Zero();
	static const BigNumber& One();
	static const BigNumber& Two();

	// arithmetic operators probably need
	BigNumber& operator = (const BigNumber& bn);
	BigNumber& operator += (const BigNumber& bn);
	BigNumber& operator -= (const BigNumber& bn);
	BigNumber& operator *= (Ipp32u n);
	BigNumber& operator *= (const BigNumber& bn);
	BigNumber& operator /= (const BigNumber& bn);
	BigNumber& operator %= (const BigNumber& bn);
	friend BigNumber operator + (const BigNumber& a, const BigNumber& b);
	friend BigNumber operator - (const BigNumber& a, const BigNumber& b);
	friend BigNumber operator * (const BigNumber& a, const BigNumber& b);
	friend BigNumber operator * (const BigNumber& a, Ipp32u);
	friend BigNumber operator % (const BigNumber& a, const BigNumber& b);
	friend BigNumber operator / (const BigNumber& a, const BigNumber& b);

	// modulo arithmetic
	BigNumber Modulo(const BigNumber& a) const;
	BigNumber ModAdd(const BigNumber& a, const BigNumber& b) const;
	BigNumber ModSub(const BigNumber& a, const BigNumber& b) const;
	BigNumber ModMul(const BigNumber& a, const BigNumber& b) const;
	BigNumber InverseAdd(const BigNumber& a) const;
	BigNumber InverseMul(const BigNumber& a) const;

	// comparisons
	friend bool operator < (const BigNumber& a, const BigNumber& b);
	friend bool operator > (const BigNumber& a, const BigNumber& b);
	friend bool operator == (const BigNumber& a, const BigNumber& b);
	friend bool operator != (const BigNumber& a, const BigNumber& b);
	friend bool operator <= (const BigNumber& a, const BigNumber& b) { return !(a > b); }
	friend bool operator >= (const BigNumber& a, const BigNumber& b) { return !(a < b); }

	// easy tests
	bool IsOdd() const;
	bool IsEven() const { return !IsOdd(); }

	// size of BigNumber
	int MSB() const;
	int LSB() const;
	int BitSize() const { return MSB() + 1; }
	int DwordSize() const { return (BitSize() + 31) >> 5; }
	friend int Bit(const vector<Ipp32u>& v, int n);

	// conversion and output
	void num2hex(string& s) const; // convert to hex string
	void num2vec(vector<Ipp32u>& v) const; // convert to 32-bit word vector
	friend ostream& operator << (ostream& os, const BigNumber& a);

	// defined function
	void tBN(const char* Msg);

protected:
	bool create(const Ipp32u* pData, int length, IppsBigNumSGN sgn = IppsBigNumPOS);
	int compare(const BigNumber&) const;
	IppsBigNumState* m_pBN;
	
};

// convert bit size into 32-bit words
#define BITSIZE_WORD(n) ((((n)+31)>>5))

#endif // _BIGNUMBER_H_



//////////////////////////////////////////////////////////////////////
//
// BigNumber
//
//////////////////////////////////////////////////////////////////////

void BigNumber::tBN(const char* Msg) {
	/*
		This function prints a representation of IPP BigNum Object
	*/
	//get state of BigNum
	const IppsBigNumState* BNR = this->m_pBN;

	int sBNR; // size of BigNum
	ippsGetSize_BN(BNR, &sBNR); // getting size
	IppsBigNumSGN sgn; // sign of BigNum
	Ipp32u* dBNR = new Ipp32u[sBNR]; // BNR data
	ippsGet_BN(&sgn, &sBNR, dBNR, BNR); // getting BNR sign and data
	int size = sBNR;

	
	BigNumber BigNum(dBNR, size, sgn); // initial  BigNum

	IppsBigNumState* BN = BigNum.Ctx(); // create(dBNR, size, sgn);// neglecting sign

	Ipp8u* vBN = new Ipp8u[size * 4]; // which is typed below
	ippsGetOctString_BN(vBN, size * 4, BN);
	if (Msg)
		cout << Msg; // header
	cout.fill('0');
	cout << hex;
	if (sgn == 0) cout << "-"; // sign
	for (int n = 0; n < size * 4; n++) {
		cout.width(2);
		cout << (int)vBN[n]; // value
	}
	cout << dec;
	cout.fill(' ');
	cout << endl;

	ippFree((Ipp8u*)BN);
	ippFree(vBN);

}

BigNumber::~BigNumber()
{
	delete[](Ipp8u*)m_pBN;
}

bool BigNumber::create(const Ipp32u* pData, int length, IppsBigNumSGN sgn)
{
	int size;
	ippsBigNumGetSize(length, &size);
	m_pBN = (IppsBigNumState*)(new Ipp8u[size]);
	if (!m_pBN)
		return false;
	ippsBigNumInit(length, m_pBN);
	if (pData)
		ippsSet_BN(sgn, length, pData, m_pBN);
	return true;
}

// constructors
//
BigNumber::BigNumber(Ipp32u value)
{
	create(&value, 1, IppsBigNumPOS);
}

BigNumber::BigNumber(Ipp32s value)
{
	Ipp32s avalue = abs(value);
	create((Ipp32u*)&avalue, 1, (value < 0) ? IppsBigNumNEG : IppsBigNumPOS);
}

BigNumber::BigNumber(const IppsBigNumState* pBN)
{
	IppsBigNumSGN bnSgn;
	int bnBitLen;
	Ipp32u* bnData;
	ippsRef_BN(&bnSgn, &bnBitLen, &bnData, pBN);

	create(bnData, BITSIZE_WORD(bnBitLen), bnSgn);
}

BigNumber::BigNumber(const Ipp32u* pData, int length, IppsBigNumSGN sgn)
{
	create(pData, length, sgn);
}

static char HexDigitList[] = "0123456789ABCDEF";

BigNumber::BigNumber(const char* s)
{
	bool neg = '-' == s[0];
	if (neg) s++;
	bool hex = ('0' == s[0]) && (('x' == s[1]) || ('X' == s[1]));

	int dataLen;
	Ipp32u base;
	if (hex) {
		s += 2;
		base = 0x10;
		dataLen = (int)(strlen(s) + 7) / 8;
	}
	else {
		base = 10;
		dataLen = (int)(strlen(s) + 9) / 10;
	}

	create(0, dataLen);
	*(this) = Zero();
	while (*s) {
		char tmp[2] = { s[0],0 };
		Ipp32u digit = (Ipp32u)strcspn(HexDigitList, tmp);
		*this = (*this) * base + BigNumber(digit);
		s++;
	}

	if (neg)
		(*this) = Zero() - (*this);
}

BigNumber::BigNumber(const BigNumber& bn)
{
	IppsBigNumSGN bnSgn;
	int bnBitLen;
	Ipp32u* bnData;
	ippsRef_BN(&bnSgn, &bnBitLen, &bnData, bn);

	create(bnData, BITSIZE_WORD(bnBitLen), bnSgn);
}

// set value
//
void BigNumber::Set(const Ipp32u* pData, int length, IppsBigNumSGN sgn)
{
	ippsSet_BN(sgn, length, pData, BN(*this));
}

// constants
//
const BigNumber& BigNumber::Zero()
{
	static const BigNumber zero(0);
	return zero;
}

const BigNumber& BigNumber::One()
{
	static const BigNumber one(1);
	return one;
}

const BigNumber& BigNumber::Two()
{
	static const BigNumber two(2);
	return two;
}

// arithmetic operators
//
BigNumber& BigNumber::operator =(const BigNumber& bn)
{
	if (this != &bn) {    // prevent self copy
		IppsBigNumSGN bnSgn;
		int bnBitLen;
		Ipp32u* bnData;
		ippsRef_BN(&bnSgn, &bnBitLen, &bnData, bn);

		delete (Ipp8u*)m_pBN;
		create(bnData, BITSIZE_WORD(bnBitLen), bnSgn);
	}
	return *this;
}

BigNumber& BigNumber::operator += (const BigNumber& bn)
{
	int aBitLen;
	ippsRef_BN(NULL, &aBitLen, NULL, *this);
	int bBitLen;
	ippsRef_BN(NULL, &bBitLen, NULL, bn);
	int rBitLen = IPP_MAX(aBitLen, bBitLen) + 1;

	BigNumber result(0, BITSIZE_WORD(rBitLen));
	ippsAdd_BN(*this, bn, result);
	*this = result;
	return *this;
}

BigNumber& BigNumber::operator -= (const BigNumber& bn)
{
	int aBitLen;
	ippsRef_BN(NULL, &aBitLen, NULL, *this);
	int bBitLen;
	ippsRef_BN(NULL, &bBitLen, NULL, bn);
	int rBitLen = IPP_MAX(aBitLen, bBitLen);

	BigNumber result(0, BITSIZE_WORD(rBitLen));
	ippsSub_BN(*this, bn, result);
	*this = result;
	return *this;
}

BigNumber& BigNumber::operator *= (const BigNumber& bn)
{
	int aBitLen;
	ippsRef_BN(NULL, &aBitLen, NULL, *this);
	int bBitLen;
	ippsRef_BN(NULL, &bBitLen, NULL, bn);
	int rBitLen = aBitLen + bBitLen;

	BigNumber result(0, BITSIZE_WORD(rBitLen));
	ippsMul_BN(*this, bn, result);
	*this = result;
	return *this;
}

BigNumber& BigNumber::operator *= (Ipp32u n)
{
	int aBitLen;
	ippsRef_BN(NULL, &aBitLen, NULL, *this);

	BigNumber result(0, BITSIZE_WORD(aBitLen + 32));
	BigNumber bn(n);
	ippsMul_BN(*this, bn, result);
	*this = result;
	return *this;
}

BigNumber& BigNumber::operator %= (const BigNumber& bn)
{
	BigNumber remainder(bn);
	ippsMod_BN(BN(*this), BN(bn), BN(remainder));
	*this = remainder;
	return *this;
}

BigNumber& BigNumber::operator /= (const BigNumber& bn)
{
	BigNumber quotient(*this);
	BigNumber remainder(bn);
	ippsDiv_BN(BN(*this), BN(bn), BN(quotient), BN(remainder));
	*this = quotient;
	return *this;
}

BigNumber operator + (const BigNumber& a, const BigNumber& b)
{
	BigNumber r(a);
	return r += b;
}

BigNumber operator - (const BigNumber& a, const BigNumber& b)
{
	BigNumber r(a);
	return r -= b;
}

BigNumber operator * (const BigNumber& a, const BigNumber& b)
{
	BigNumber r(a);
	return r *= b;
}

BigNumber operator * (const BigNumber& a, Ipp32u n)
{
	BigNumber r(a);
	return r *= n;
}

BigNumber operator / (const BigNumber& a, const BigNumber& b)
{
	BigNumber q(a);
	return q /= b;
}

BigNumber operator % (const BigNumber& a, const BigNumber& b)
{
	BigNumber r(b);
	ippsMod_BN(BN(a), BN(b), BN(r));
	return r;
}

// modulo arithmetic
//
BigNumber BigNumber::Modulo(const BigNumber& a) const
{
	return a % *this;
}

BigNumber BigNumber::InverseAdd(const BigNumber& a) const
{
	BigNumber t = Modulo(a);
	if (t == BigNumber::Zero())
		return t;
	else
		return *this - t;
}

BigNumber BigNumber::InverseMul(const BigNumber& a) const
{
	BigNumber r(*this);
	ippsModInv_BN(BN(a), BN(*this), BN(r));
	return r;
}

BigNumber BigNumber::ModAdd(const BigNumber& a, const BigNumber& b) const
{
	BigNumber r = this->Modulo(a + b);
	return r;
}


BigNumber BigNumber::ModSub(const BigNumber& a, const BigNumber& b) const
{
	BigNumber r = this->Modulo(a + this->InverseAdd(b));
	return r;
}

BigNumber BigNumber::ModMul(const BigNumber& a, const BigNumber& b) const
{
	BigNumber r = this->Modulo(a*b);
	return r;
}

// comparison
//
int BigNumber::compare(const BigNumber &bn) const
{
	Ipp32u result;
	BigNumber tmp = *this - bn;
	ippsCmpZero_BN(BN(tmp), &result);
	return (result == IS_ZERO) ? 0 : (result == GREATER_THAN_ZERO) ? 1 : -1;
}

bool operator <  (const BigNumber &a, const BigNumber &b) { return a.compare(b) < 0; }
bool operator >  (const BigNumber &a, const BigNumber &b) { return a.compare(b) > 0; }
bool operator == (const BigNumber &a, const BigNumber &b) { return 0 == a.compare(b); }
bool operator != (const BigNumber &a, const BigNumber &b) { return 0 != a.compare(b); }

// easy tests
//
bool BigNumber::IsOdd() const
{
	Ipp32u* bnData;
	ippsRef_BN(NULL, NULL, &bnData, *this);
	return bnData[0] & 1;
}

// size of BigNumber
//
int BigNumber::LSB() const
{
	if (*this == BigNumber::Zero())
		return 0;

	vector<Ipp32u> v;
	num2vec(v);

	int lsb = 0;
	vector<Ipp32u>::iterator i;
	for (i = v.begin(); i != v.end(); i++) {
		Ipp32u x = *i;
		if (0 == x)
			lsb += 32;
		else {
			while (0 == (x & 1)) {
				lsb++;
				x >>= 1;
			}
			break;
		}
	}
	return lsb;
}

int BigNumber::MSB() const
{
	if (*this == BigNumber::Zero())
		return 0;

	vector<Ipp32u> v;
	num2vec(v);

	int msb = (int)v.size() * 32 - 1;
	vector<Ipp32u>::reverse_iterator i;
	for (i = v.rbegin(); i != v.rend(); i++) {
		Ipp32u x = *i;
		if (0 == x)
			msb -= 32;
		else {
			while (!(x & 0x80000000)) {
				msb--;
				x <<= 1;
			}
			break;
		}
	}
	return msb;
}

int Bit(const vector<Ipp32u>& v, int n)
{
	return 0 != (v[n >> 5] & (1 << (n & 0x1F)));
}

// conversions and output
//
void BigNumber::num2vec(vector<Ipp32u>& v) const
{
	int bnBitLen;
	Ipp32u* bnData;
	ippsRef_BN(NULL, &bnBitLen, &bnData, *this);

	int len = BITSIZE_WORD(bnBitLen);;
	for (int n = 0; n < len; n++)
		v.push_back(bnData[n]);
}

void BigNumber::num2hex(string& s) const
{
	IppsBigNumSGN bnSgn;
	int bnBitLen;
	Ipp32u* bnData;
	ippsRef_BN(&bnSgn, &bnBitLen, &bnData, *this);

	int len = BITSIZE_WORD(bnBitLen);

	s.append(1, (bnSgn == ippBigNumNEG) ? '-' : ' ');
	s.append(1, '0');
	s.append(1, 'x');
	for (int n = len; n > 0; n--) {
		Ipp32u x = bnData[n - 1];
		for (int nd = 8; nd > 0; nd--) {
			char c = HexDigitList[(x >> (nd - 1) * 4) & 0xF];
			s.append(1, c);
		}
	}
}

ostream& operator << (ostream &os, const BigNumber& a)
{
	string s;
	a.num2hex(s);
	os << s.c_str();
	return os;
}

////////////////////////////////
// Ends BigNum functions
//////////////////////////////





////////////////////////////////
// Functions for Prime Num generators 
//////////////////////////////

#if !defined _CPOBJS_H_
#define _CPOBJS_H_

//
// create new of some ippCP 'objects'
//
//#include "ippcp.h"
//#include <stdlib.h>

#define BITS_2_WORDS(n) (((n)+31)>>5)
int Bitsize2Wordsize(int nBits);

Ipp32u* rand32(Ipp32u* pX, int size);

IppsBigNumState* newBN(int len, const Ipp32u* pData = 0);
IppsBigNumState* newRandBN(int len);
void deleteBN(IppsBigNumState* pBN);

IppsPRNGState* newPRNG(int seedBitsize = 160);
void deletePRNG(IppsPRNGState* pPRNG);

IppsPrimeState* newPrimeGen(int seedBitsize = 160);
void deletePrimeGen(IppsPrimeState* pPrime);

//IppsRSAState* newRSA(int lenN, int lenP, IppRSAKeyType type);
//void deleteRSA(IppsRSAState* pRSA);

IppsDLPState* newDLP(int lenM, int lenL);
void deleteDLP(IppsDLPState* pDLP);

#endif // _CPOBJS_H_


// convert bitsize into 32-bit wordsize
int Bitsize2Wordsize(int nBits)
{
	return (nBits + 31) >> 5;
}

// new BN number
IppsBigNumState* newBN(int len, const Ipp32u* pData)
{
	int size;
	ippsBigNumGetSize(len, &size);
	IppsBigNumState* pBN = (IppsBigNumState*)(new Ipp8u[size]);
	ippsBigNumInit(len, pBN);
	if (pData)
		ippsSet_BN(IppsBigNumPOS, len, pData, pBN);
	return pBN;
}
void deleteBN(IppsBigNumState* pBN)
{
	delete[](Ipp8u*)pBN;
}

// set up array of 32-bit items random
Ipp32u* rand32(Ipp32u* pX, int size)
{
	for (int n = 0; n < size; n++)
		pX[n] = (rand() << 16) + rand();
	return pX;
}

IppsBigNumState* newRandBN(int len)
{
	Ipp32u* pBuffer = new Ipp32u[len];
	IppsBigNumState* pBN = newBN(len, rand32(pBuffer, len));
	delete[] pBuffer;
	return pBN;
}

//
// 'external' PRNG
//
IppsPRNGState* newPRNG(int seedBitsize)
{
	int seedSize = Bitsize2Wordsize(seedBitsize);
	Ipp32u* seed = new Ipp32u[seedSize];
	Ipp32u* augm = new Ipp32u[seedSize];

	int size;
	IppsBigNumState* pTmp;
	ippsPRNGGetSize(&size);
	IppsPRNGState* pCtx = (IppsPRNGState*)(new Ipp8u[size]);
	ippsPRNGInit(seedBitsize, pCtx);

	ippsPRNGSetSeed(pTmp = newBN(seedSize, rand32(seed, seedSize)), pCtx);
	delete[](Ipp8u*)pTmp;
	ippsPRNGSetAugment(pTmp = newBN(seedSize, rand32(augm, seedSize)), pCtx);
	delete[](Ipp8u*)pTmp;

	delete[] seed;
	delete[] augm;
	return pCtx;
}
void deletePRNG(IppsPRNGState* pPRNG)
{
	delete[](Ipp8u*)pPRNG;
}

//
// Prime Generator context
//
IppsPrimeState* newPrimeGen(int maxBits)
{
	int size;
	ippsPrimeGetSize(maxBits, &size);
	IppsPrimeState* pCtx = (IppsPrimeState*)(new Ipp8u[size]);
	ippsPrimeInit(maxBits, pCtx);
	return pCtx;
}
void deletePrimeGen(IppsPrimeState* pPrimeG)
{
	delete[](Ipp8u*)pPrimeG;
}

//
// RSA context
//
/*IppsRSAState* newRSA(int lenN, int lenP, IppRSAKeyType type)
{
	int size;
	ippsRSAGetSize(lenN, lenP, type, &size);
	IppsRSAState* pCtx = (IppsRSAState*)(new Ipp8u[size]);
	ippsRSAInit(lenN, lenP, type, pCtx);
	return pCtx;
}*/
/*void deleteRSA(IppsRSAState* pRSA)
{
	delete[](Ipp8u*)pRSA;
}*/

//
// DLP context
//
IppsDLPState* newDLP(int lenM, int lenL)
{
	int size;
	ippsDLPGetSize(lenM, lenL, &size);
	IppsDLPState *pCtx = (IppsDLPState *)new Ipp8u[size];
	ippsDLPInit(lenM, lenL, pCtx);
	return pCtx;
}
void deleteDLP(IppsDLPState* pDLP)
{
	delete[](Ipp8u*)pDLP;
}

//////////////////////////////////////////////////////
// End prime Num generator functions
////////////////////////////////////////////////////








// P prime factor
BigNumber    P("0xEECFAE81B1B9B3C908810B10A1B5600199EB9F44AEF4FDA493B81A9E3D84F632"
	"124EF0236E5D1E3B7E28FAE7AA040A2D5B252176459D1F397541BA2A58FB6599");
// Q prime factor
BigNumber    Q("0xC97FB1F027F453F6341233EAAAD1D9353F6C42D08866B1D05A0F2035028B9D86"
	"9840B41666B42E92EA0DA3B43204B5CFCE3352524D0416A5A441E700AF461503");
// P's CRT exponent
BigNumber   dP("0x54494CA63EBA0337E4E24023FCD69A5AEB07DDDC0183A4D0AC9B54B051F2B13E"
	"D9490975EAB77414FF59C1F7692E9A2E202B38FC910A474174ADC93C1F67C981");
// Q's CRT exponent
BigNumber   dQ("0x471E0290FF0AF0750351B7F878864CA961ADBD3A8A7E991C5C0556A94C3146A7"
	"F9803F8F6F8AE342E931FD8AE47A220D1B99A495849807FE39F9245A9836DA3D");
// CRT coefficient
BigNumber invQ("0xB06C4FDABB6301198D265BDBAE9423B380F271F73453885093077FCD39E2119F"
	"C98632154F5883B167A967BF402B4E9E2E0F9656E698EA3666EDFB25798039F7");
// rsa modulus N = P*Q
BigNumber    N("0xBBF82F090682CE9C2338AC2B9DA871F7368D07EED41043A440D6B6F07454F51F"
	"B8DFBAAF035C02AB61EA48CEEB6FCD4876ED520D60E1EC4619719D8A5B8B807F"
	"AFB8E0A3DFC737723EE6B4B7D93A2584EE6A649D060953748834B2454598394E"
	"E0AAB12D7B61A51F527A9A41F6C1687FE2537298CA2A8F5946F8E5FD091DBDCB");
// private exponent
BigNumber    D("0xA5DAFC5341FAF289C4B988DB30C1CDF83F31251E0668B42784813801579641B2"
	"9410B3C7998D6BC465745E5C392669D6870DA2C082A939E37FDCB82EC93EDAC9"
	"7FF3AD5950ACCFBC111C76F1A9529444E56AAF68C56C092CD38DC3BEF5D20A93"
	"9926ED4F74A13EDDFBE1A1CECC4894AF9428C2B7B8883FE4463A4BC85B1CB3C1");
// public exponent
BigNumber    E("0x11");

int main() {
	BigNumber  D("0xA5DAFC5341FAF289C4B988DB30C1CDF83F31251E0668B42784813801579641B2"
		"9410B3C7998D6BC465745E5C392669D6870DA2C082A939E37FDCB82EC93EDAC9"
		"7FF3AD5950ACCFBC111C76F1A9529444E56AAF68C56C092CD38DC3BEF5D20A93"
		"9926ED4F74A13EDDFBE1A1CECC4894AF9428C2B7B8883FE4463A4BC85B1CB3C1");

	D.tBN("Representation of Data: ");
	getchar();
}

int RSA_sample()
{
	int keyCtxSize;

	// (bit) size of key components
	int bitsN = N.BitSize();
	int bitsE = E.BitSize();
	int bitsP = P.BitSize();
	int bitsQ = Q.BitSize();

	// define and setup public key
	ippsRSA_GetSizePublicKey(bitsN, bitsE, &keyCtxSize);
	IppsRSAPublicKeyState* pPub = (IppsRSAPublicKeyState*)(new Ipp8u[keyCtxSize]);
	ippsRSA_InitPublicKey(bitsN, bitsE, pPub, keyCtxSize);
	ippsRSA_SetPublicKey(N, E, pPub);

	// define and setup (type2) private key
	ippsRSA_GetSizePrivateKeyType2(bitsP, bitsQ, &keyCtxSize);
	IppsRSAPrivateKeyState* pPrv = (IppsRSAPrivateKeyState*)(new Ipp8u[keyCtxSize]);
	ippsRSA_InitPrivateKeyType2(bitsP, bitsQ, pPrv, keyCtxSize);
	ippsRSA_SetPrivateKeyType2(P, Q, dP, dQ, invQ, pPrv);

	// allocate scratch buffer
	int buffSizePublic;
	ippsRSA_GetBufferSizePublicKey(&buffSizePublic, pPub);
	int buffSizePrivate;
	ippsRSA_GetBufferSizePrivateKey(&buffSizePrivate, pPrv);
	int buffSize = buffSizePublic; //max(buffSizePublic, buffSizePrivate);
	Ipp8u* scratchBuffer = NULL;
	scratchBuffer = new Ipp8u[buffSize];

	// error flag
	int error = 0;

	do {
		//
		// validate keys
		//

		// random generator
		IppsPRNGState* pRand = newPRNG();
		// prime generator
		IppsPrimeState* pPrimeG = newPrimeGen(P.BitSize());

		int validateRes = IPP_IS_INVALID;
		ippsRSA_ValidateKeys(&validateRes,
			pPub, pPrv, NULL, scratchBuffer,
			10, pPrimeG, ippsPRNGen, pRand);

		// delete geterators
		deletePrimeGen(pPrimeG);
		deletePRNG(pRand);

		if (IPP_IS_VALID != validateRes) {
			cout << "validation fail" << endl;
			error = 1;
			break;
		}

		// known plain- and ciper-texts
		BigNumber kat_PT("0x00EB7A19ACE9E3006350E329504B45E2CA82310B26DCD87D5C68F1EEA8F55267"
			"C31B2E8BB4251F84D7E0B2C04626F5AFF93EDCFB25C9C2B3FF8AE10E839A2DDB"
			"4CDCFE4FF47728B4A1B7C1362BAAD29AB48D2869D5024121435811591BE392F9"
			"82FB3E87D095AEB40448DB972F3AC14F7BC275195281CE32D2F1B76D4D353E2D");

		BigNumber kat_CT("0x1253E04DC0A5397BB44A7AB87E9BF2A039A33D1E996FC82A94CCD30074C95DF7"
			"63722017069E5268DA5D1C0B4F872CF653C11DF82314A67968DFEAE28DEF04BB"
			"6D84B1C31D654A1970E5783BD6EB96A024C2CA2F4A90FE9F2EF5C9C140E5BB48"
			"DA9536AD8700C84FC9130ADEA74E558D51A74DDF85D8B50DE96838D6063E0955");

		//
		// encrypt  message
		//
		BigNumber ct(0, N.DwordSize());
		ippsRSA_Encrypt(kat_PT, ct, pPub, scratchBuffer);
		if (ct != kat_CT) {
			cout << "encryption fail" << endl;
			error = 1;
			break;
		}

		//
		// decrypt message
		//
		BigNumber rt(0, N.DwordSize());
		ippsRSA_Decrypt(kat_CT, rt, pPrv, scratchBuffer);
		if (rt != kat_PT) {
			cout << "decryption fail" << endl;
			error = 1;
			break;
		}
	} while (0);

	delete[] scratchBuffer;

	delete[](Ipp8u*) pPub;

	// remove sensitive data before release
	ippsRSA_InitPrivateKeyType2(bitsP, bitsQ, pPrv, keyCtxSize);
	delete[](Ipp8u*) pPrv;

	return error == 0;
}