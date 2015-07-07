/*
 * Expert.h
 *
 *  Created on: 2011-12-25
 *      Author: Administrator
 */

#ifndef EXPERT_H_
#define EXPERT_H_

namespace std {

class Expert {
public:
	Expert();
	virtual ~Expert();
	static void OnRecvOptionsRqst_u(ISipCall * call, ISipMessage * request);
	static void OnRecvByeRqst_u(ISipCall * call, ISipMessage * request);
private:
	void constructLocalSdp(ISipMessage* response);
};

} /* namespace std */
#endif /* EXPERT_H_ */
