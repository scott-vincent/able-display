#ifndef _SEVENSEGMENT_H_
#define _SEVENSEGMENT_H_

class sevensegment
{
private:
	int channel;
	unsigned char prevDisplay[4][8];	// 4 displays daisy chained

public:
	sevensegment(bool initWiringPi, int spiChannel);
	void dim(int display, bool wantDim);
	void getSegData(unsigned char* buf, int bufSize, int num, int fixedSize);
	void getSegDegrees(unsigned char* buf, int bufSize, int numX10);
	void blankSegData(unsigned char* buf, int bufSize, bool wantMinus);
        void error(unsigned char *buf, int errNum);
	void decimalSegData(unsigned char* buf, int pos);
	void writeSegData4(unsigned char* buf1, unsigned char* buf2, unsigned char* buf3, unsigned char* buf4);

private:
	void writeSegHex(int display, char* hex);
	void writeSegData(int display, unsigned char* buf);
};

#endif // _SEVENSEGMENT_H_
