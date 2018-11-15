#include "ippcp.h"
#include "ippcore.h"

#include <stdlib.h>
#include <iostream>
#include <stdio.h>

#include <vector>
#include <iterator>

using namespace std;



/////////////////////////////////////////////////////////////////////////////////////
// functions for manipulating Big integer numbers
// MOST OF THESE FUNCTIONS ARE INTELL IPP SOURCE CODE
//HOWEVER A FEW FUNCTIONS ARE MODIFIED OR INCLUDED TO SUIT MY IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////////////

#if !defined _BIGNUMBER_H_
#define _BIGNUMBER_H_

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

	IppsBigNumState* Ctx() const { return m_pBN; } // NOT INTELL SOURCE CODE

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
	void tBN(const char* Msg); // NOT INTEL IPP CODE

protected:
	bool create(const Ipp32u* pData, int length, IppsBigNumSGN sgn = IppsBigNumPOS);
	int compare(const BigNumber&) const;
	IppsBigNumState* m_pBN;
	
}; 

// convert bit size into 32-bit words
#define BITSIZE_WORD(n) ((((n)+31)>>5))
#endif // _BIGNUMBER_H_


// implementation of BigNum object classes defined above
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

// constructors of BigNum

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
////////////////////////////////////////////////////
// END BIGNUM FUNCTIONS IMPLEMENTATIONS
//////////////////////////////////////////////////





/////////////////////////////////////////////////
// Functions for Prime Num generators 
////////////////////////////////////////////////

#if !defined _CPOBJS_H_
#define _CPOBJS_H_


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


IppsDLPState* newDLP(int lenM, int lenL);
void deleteDLP(IppsDLPState* pDLP);

#endif // _CPOBJS_H_

// implement abstract functions above

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
/////////////////////////////////////////////////////









//=========MAIN RSA PROGRAM OPTIMIZED WITH IPP CRYPTOGRAPHIC FUNCTIONS=============//

int RSA_sample()
{
	int keyCtxSize;

	Ipp8u * scratchBuffer = NULL;

	Ipp32u E = { 0x11 };
	IppsBigNumState* pSrcPublicExp = newBN(1, &E);
	IppsBigNumState* pModulus = newBN(1024 / 32, NULL);
	IppsBigNumState* pPublicExp = newBN(1024 / 32, NULL);
	IppsBigNumState* pPrivateExp = newBN(1024 / 32, NULL);

	// (bit) size of key components
	int bitsN = 1024;
	int bitsE = 512;
	int bitsP = 512;
	int bitsQ = 512;

	// define and setup public key
	ippsRSA_GetSizePublicKey(bitsN, bitsE, &keyCtxSize);
	IppsRSAPublicKeyState* pPub = (IppsRSAPublicKeyState*)(new Ipp8u[keyCtxSize]);
	ippsRSA_InitPublicKey(bitsN, bitsE, pPub, keyCtxSize);

	// define and setup (type2) private key
	ippsRSA_GetSizePrivateKeyType2(bitsP, bitsQ, &keyCtxSize);
	IppsRSAPrivateKeyState* pPrv = (IppsRSAPrivateKeyState*)(new Ipp8u[keyCtxSize]);
	ippsRSA_InitPrivateKeyType2(bitsP, bitsQ, pPrv, keyCtxSize);

	// allocate scratch buffer
	int buffSizePublic;
	ippsRSA_GetBufferSizePublicKey(&buffSizePublic, pPub);
	int buffSizePrivate;
	ippsRSA_GetBufferSizePrivateKey(&buffSizePrivate, pPrv);
	int buffSize = buffSizePrivate; 
	scratchBuffer = new Ipp8u[buffSize];


	// random generator
	IppsPRNGState* pRand = newPRNG();

	// prime generator
	IppsPrimeState* pPrimeG = newPrimeGen(512);


	// validate keys
	int validateRes = IS_VALID_KEY;
	ippsRSA_ValidateKeys(&validateRes,
		pPub, pPrv, NULL, scratchBuffer,
		10, pPrimeG, ippsPRNGen, pRand);

	if (IS_VALID_KEY == validateRes) {
		cout << "validation successful" << endl;
	}

	// keys generator
	IppStatus status;
	status = ippsRSA_GenerateKeys(pSrcPublicExp, pModulus, pPublicExp, pPrivateExp, pPrv, scratchBuffer,
		10, pPrimeG, ippsPRNGen, pRand);

	// check for successfull generation of keys
	if (status == ippStsNoErr) {
		cout << "keys generation successful" << endl;
	}

	// delete generators
	deletePrimeGen(pPrimeG);
	deletePRNG(pRand);


	// Retrieving Components of RSA states

	// get modulus generated
	BigNumber modN(pModulus);
	modN.tBN("Modulus: ");

	// get public key generated 
	BigNumber Pk(pPublicExp);
	Pk.tBN("Public Key Exponent (e): ");

	//get private key generated
	BigNumber Pvk(pPrivateExp);
	Pvk.tBN("Private Key Exponent (d): ");

	// public key components retrieved 
	ippsRSA_SetPublicKey(pModulus, pPublicExp, pPub);
	// set up type1 private key components
	ippsRSA_SetPrivateKeyType1(pModulus, pPrivateExp, pPrv);

	// Encrypt AND Decrypt Message

	Ipp32u dataM[] = { // plain text
		0x12345678,0xabcde123,0x87654321,
		0x111aaaa2,0xbbbbbbbb,0xcccccccc,
		0x12237777,0x82234587,0x1ef392c9,
		0x43581159,0xb5024121,0xa48D2869,
		0x2abababa,0x1a2b3c22,0xa47728B4,
		0x54321123,0xaaaaaaaa,0xbbbbbbbb,
		0xcccccccc,0xdddddddd,0x34667666,
		0xa46a3aaa,0xe4251e84,0xf31f2Eff,
		0xfec55267,0x11111111,0x98765432,
		0x54376511,0x21323111,0x85433abc,0xcaa44322,0x001234ef };

	Ipp32u dataN[] = { // data for ciphertext context creation
		0x03cccb37,0x6acadded,0xdf4f20d0,0x2458257d,
		0xda3b7886,0x5c1b1a4c,0xea6f676b,0x59f51e09,
		0xc0691195,0x8076c61f,0x4221d059,0xd021673a,
		0x139bd5ef,0x95189046,0x10eb90ea,0x127af4e5,
		0x14f5dcb8,0x1e13510f,0x6e2e0558,0xa650fce0,
		0xff0bcd51,0xe218e43d,0xad045536,0xdc4a21d7,
		0x74edee68,0xb474ad57,0x79514004,0xa65a27a3,0x9e5259c1,0xe78e89eb,
		0xb34ed292,0x99197f0d };

	// create contexts
	IppsBigNumState* Msg = newBN(sizeof(dataM) / sizeof(dataM[0]), dataM); // create message context
	IppsBigNumState* C = newBN(sizeof(dataN) / sizeof(dataN[0]));   // create ciphertext context
	IppsBigNumState* Z = newBN(sizeof(dataN) / sizeof(dataN[0]));   // create de-ciphertext context

	//
	// encrypt  message
	//
	IppStatus status1;
	status1 = ippsRSA_Encrypt(Msg, C, pPub, scratchBuffer);

	// check for successfull encryption of msg
	if (status1 == ippStsNoErr) {
		cout << "message encryption successful" << endl;
	}

	//
	// de-crypt  message
	//
	IppStatus status2;
	status2 = ippsRSA_Decrypt(C, Z, pPrv, scratchBuffer);

	// check for successfull encryption of msg
	if (status2 == ippStsNoErr) {
		cout << "message decryption successful" << endl;
	}

	// compare plaintext and decrypted message
	Ipp32u Result;
	ippsCmp_BN(Msg, Z, &Result); // plain text and decrypted cipher text
	cout << Result <<endl; // comparison 0 --> OK

	// remove sensitive data before release
	ippsRSA_InitPrivateKeyType2(bitsP, bitsQ, pPrv, keyCtxSize);

	// release resource
	delete[] scratchBuffer;
	delete[](Ipp8u*) pPub;
	delete[](Ipp8u*) pPrv;
	
	return 0;
}

int main() { // run program
	
	RSA_sample(); // run RSA program implemented
	getchar(); // pause program to view info
	
	return 0;
}