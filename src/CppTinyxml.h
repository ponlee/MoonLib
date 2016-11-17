#ifndef _CPP_TINYXML_H_
#define _CPP_TINYXML_H_

#include <string>

#include <tinyxml.h>

#include "CppLog.h"

using std::string;

class CppTinyXml
{
public:
    static const TiXmlNode *GetXmlNode(const TiXmlNode *xmlNodeParant, const string &name) throw(CppException);

    static TiXmlNode *GetXmlNode(TiXmlNode *xmlNodeParant, const string &name) throw(CppException);

    //************************************
    // Describe:  获得节点文字
    // Parameter: const TiXmlNode * xmlNodeParant
    // Parameter: const string & name
    // Parameter: bool isThrow      如果为true并且获得失败，则抛异常，否则返回""
    // Returns:   string            获得的节点文字
    // Author:    moontan
    //************************************
    static string GetXmlText(const TiXmlNode *xmlNodeParant, const string &name, bool emptyThrow = false) throw(CppException);

    static string GetXmlAttribute(const TiXmlElement *xmlElementParant, const string &name);

    static TiXmlElement *AddXmlElement(TiXmlNode *xmlParent, const string &name);

    static TiXmlElement *AddXmlText(TiXmlNode *xmlParent, const string &key, const string &value);

    static TiXmlText *SetXmlText(TiXmlNode *xmlParent, const string &value);

    static string ParseXml(const TiXmlNode *xmlRsp, bool needFormat = true);
};


#endif
