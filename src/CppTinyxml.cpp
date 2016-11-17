#include "CppTinyxml.h"


const TiXmlNode *CppTinyXml::GetXmlNode(const TiXmlNode *xmlNodeParant, const string &name) throw(CppException)
{
    const TiXmlNode *xmlNode = xmlNodeParant->FirstChild(name.c_str());
    if (xmlNode == NULL)
    {
        THROW("Can't get node[%s]", name.c_str());
    }

    return xmlNode;
}

TiXmlNode * CppTinyXml::GetXmlNode(TiXmlNode *xmlNodeParant, const string &name) throw(CppException)
{
    TiXmlNode *xmlNode = xmlNodeParant->FirstChild(name.c_str());
    if (xmlNode == NULL)
    {
        THROW("Can't get node[%s]", name.c_str());
    }

    return xmlNode;
}

string CppTinyXml::GetXmlText(const TiXmlNode *xmlNodeParant, const string &name, bool emptyThrow) throw(CppException)
{
    const TiXmlNode* tmpNode;
    if ((tmpNode = xmlNodeParant->FirstChild(name.c_str())) != NULL
        && (tmpNode = tmpNode->FirstChild()) != NULL)
    {
        return tmpNode->Value();
    }

    if (emptyThrow)
    {
        THROW("Can't get text[%s]", name.c_str());
    }

    return "";
}

string CppTinyXml::GetXmlAttribute(const TiXmlElement *xmlElementParant, const string &name)
{
    const char *tempStr;
    if ((tempStr = xmlElementParant->Attribute(name.c_str())) != NULL)
    {
        return tempStr;
    }

    return "";
}

TiXmlElement * CppTinyXml::AddXmlElement(TiXmlNode *xmlParent, const string &name)
{
    TiXmlElement *xmlChild = new TiXmlElement(name.c_str());
    xmlParent->LinkEndChild(xmlChild);
    return xmlChild;
}

TiXmlElement * CppTinyXml::AddXmlText(TiXmlNode *xmlParent, const string &key, const string &value)
{
    TiXmlElement *xmlKey = new TiXmlElement(key.c_str());
    TiXmlText *xmlValue = new TiXmlText(value.c_str());
    xmlKey->LinkEndChild(xmlValue);
    xmlParent->LinkEndChild(xmlKey);

    return xmlKey;
}

TiXmlText * CppTinyXml::SetXmlText(TiXmlNode *xmlParent, const string &value)
{
    TiXmlText *xmlValue = new TiXmlText(value.c_str());
    xmlParent->LinkEndChild(xmlValue);

    return xmlValue;
}

string CppTinyXml::ParseXml(const TiXmlNode *xmlRsp, bool needFormat /*= true*/)
{
    TiXmlPrinter xmlPrinter;
    if (!needFormat)
    {
        xmlPrinter.SetIndent("");
        xmlPrinter.SetLineBreak("");
    }

    xmlRsp->Accept(&xmlPrinter);

    return xmlPrinter.CStr();
}
