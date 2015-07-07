/*
 * CertVerification.h
 *
 *  Created on: 2012-1-3
 *      Author: Administrator
 */

#ifndef CERTVERIFICATION_H_
#define CERTVERIFICATION_H_
#define CERTVERIFICATION_ON
namespace std {

class CertVerification {
public:
	CertVerification();
	virtual ~CertVerification()
	{
		delete this;
		cert_v = NULL;
	}
	virtual void CertVerify() = 0;
	void DelInstance()
	{
		if(this != NULL)
		{
			delete this;
		}
		cert_v = NULL;
	}
	CertVerification* GetInstance()
	{
		return cert_v;
	}
private:
	static CertVerification* cert_v = NULL;
};

} /* namespace std */
#endif /* CERTVERIFICATION_H_ */
