/*
 * Expert.cpp
 *
 *  Created on: 2011-12-25
 *      Author: Administrator
 */

#include "Expert.h"

namespace std {

Expert::Expert()
{
}

Expert::~Expert()
{
}

static void Expert::OnRecvOptionsRqst_u(ISipCall * call, ISipMessage * request)
{
	ISipMessage * response = ISipMessage::CreateSipResponse(request);
	response->SetResponseStatusCode(200);
	call->Respond(response);
}

static void Expert::OnRecvByeRqst_u(ISipCall * call, ISipMessage * request)
{
	ISipMessage * response = ISipMessage::CreateSipResponse(request);
	response->SetResponseStatusCode(200);
	call->Respond(response);
}

void Expert::constructLocalSdp(ISipMessage* response)
{
	response->SetContentType("application/sdp");
	ISessionDescription* sdp = response->GetSessionDescription();
	sdp->GetOrigin()->SetAddress("10.224.42.24");
	sdp->SetInfo("info");
	sdp->SetSessionName("session");
	sdp->SetURI("uri");
	sdp->SetVersion(0);
	sdp->GetConnection()->SetAddress("10.224.42.24");
	ISessionDescription::MediaDescrContainer& mediaList = sdp->GetMediaDescriptionList();
	ISDMediaDescription* media = ISDMediaDescription::Create();
	media->SetInfo("audio");
	media->SetMediaPort(49152);
	media->SetMediaType("audio");
	media->SetProtocol("RTP/AVP");
	media->GetAttribute()->AddAttribute("test_attr", "test_value");
	vector<int>& format = media->GetPayLoadTypeList();
	format.push_back(3);
	vector<ISDCodec*>& codecs = media->GetAllCodecs();
	ISDCodec* codec = ISDCodec::Create();
	codecs.push_back(codec);
	codec->SetName("GSM");
	codec->SetPayloadType(3);
	codec->SetRate(8000);
	mediaList.push_back(media);
}

} /* namespace std */
