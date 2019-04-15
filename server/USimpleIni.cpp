#include <iostream>
#include <stdexcept>
#include "USimpleIni.h"

using namespace std;

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class SimpleIni                                                                                        ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
TSimpleIni::TSimpleIni(const string& filename) : m_OptionCommentCharacters(";#")
{
    if(filename!="")
    {
        if(!Load(filename)) throw logic_error("Unable to open the file "+filename+" in read mode.");
    }
}

TSimpleIni::~TSimpleIni()
{
    Free();
}

void TSimpleIni::SetOptions(optionKey key, const std::string& value)
{
    switch(key)
    {
        case optionKey::Comment :
            m_OptionCommentCharacters = value;
    }
}

bool TSimpleIni::Load(const string& filename)
{
    size_t pos;
    size_t pos2;
    size_t length;
    string line;
    string section;
    string key;
    string comment;
    ifstream file;
    IniLine iniLine;


    Free();
    m_FileName = filename;

    //*** Ouverture du fichier
    file.open(m_FileName.c_str(), ifstream::in);
    if(!file) return false;

    //*** Parcours du fichier
	while(getline(file, line))
    {
        ParasitCar(line);
        if(line.empty()) continue;
        length = line.length();

        //*** Raz
        key = "";
        iniLine.value = "";
        iniLine.comment = "";

        //*** Section ?
        if(line.at(0)=='[')
        {
            pos = line.find_first_of(']');
            if(pos==string::npos) pos = line.length();
            section = Trim(line.substr(1, pos-1));
            if(comment!="")
            {
                m_DescriptionMap[section][""] = comment;
                comment = "";
            }
            continue;
        }

        //*** Commentaire ?
        pos=string::npos;
        for(unsigned int i = 0; i < m_OptionCommentCharacters.length(); ++i)
        {
            pos2 = line.find_first_of(m_OptionCommentCharacters[i]);
            if(pos2==string::npos) continue;
            if(pos==string::npos)
            {
                pos=pos2;
                continue;
            }
            if(pos>pos2) pos = pos2;
        }
        if(pos!=string::npos)
        {
            if(pos>0)
            {
                iniLine.comment = line.substr(pos+1, length-pos);
                line.erase(pos, length-pos);
            }
            else
            {
                if(comment!="") comment += '\n';
                comment += line.substr(pos+1, length-pos);
                continue;
            }
        }

        //*** Valeur ?
        pos = line.find_first_of('=');
        if(pos!=string::npos)
        {
            iniLine.value = Trim(line.substr(pos+1, length-pos));
            line.erase(pos, length-pos);
        }

        //*** Mémorisation
        key = Trim(line);
        m_IniMap[section][key] = iniLine;
        if(comment!="")
        {
            m_DescriptionMap[section][key] = comment;
            comment = "";
        }

    }

    file.close();
    return true;
}

bool TSimpleIni::Save()
{
    return SaveAs(m_FileName);
}

bool TSimpleIni::SaveAs(const string& filename)
{
	std::map<std::string, std::map<std::string, TSimpleIni::IniLine> >::iterator itSection;
	std::map<std::string, TSimpleIni::IniLine>::iterator itKey;
    IniLine iniLine;
	ofstream file;
	bool first = true;

	file.open(filename.c_str());
    if(!file) return false;

	for(itSection=m_IniMap.begin(); itSection!=m_IniMap.end(); ++itSection)
	{
	    if(!first) file << endl;
	    SaveDescription(itSection->first, "", file);
		if(itSection->first!="") file << "[" << itSection->first << "]" << endl;

		for(itKey=itSection->second.begin(); itKey!=itSection->second.end(); ++itKey)
        {
            SaveDescription(itSection->first, itKey->first, file);
            iniLine = itKey->second;
            if(itKey->first != "") file << itKey->first << "=" << iniLine.value;
            if(iniLine.comment != "")
            {
                if(itKey->first != "")
                    file << "\t;";
                else
                    file << "#";
                file << iniLine.comment;
            }
            file << endl;
        }
        first = false;
	}

	file.close();

	return true;
}

void TSimpleIni::SaveDescription(string section, string key, ofstream &file)
{
    stringstream ss(m_DescriptionMap[section][key]);
    string item;
    while (std::getline(ss, item, '\n'))
    {
        file << "#" << item << endl;
    }
}

void TSimpleIni::Free()
{
    m_IniMap.clear();
}

string TSimpleIni::GetValue(const string& section, const string& key, const string& defaultValue)
{
	map<string, map<string, TSimpleIni::IniLine> >::iterator itSection=m_IniMap.find(section);
	if(itSection == m_IniMap.end()) return defaultValue;

	map<string, TSimpleIni::IniLine>::iterator itKey=itSection->second.find(key);
	if(itKey == itSection->second.end()) return defaultValue;

	return itKey->second.value;
}

void TSimpleIni::SetValue(const string& section, const string& key, const string& value)
{
    IniLine iniLine;

	iniLine = m_IniMap[section][key];
	iniLine.value = value;
	m_IniMap[section][key] = iniLine;
}

string TSimpleIni::GetComment(const string& section, const string& key)
{
	map<string, map<string, TSimpleIni::IniLine> >::iterator itSection=m_IniMap.find(section);
	if(itSection == m_IniMap.end()) return "";

	map<string, TSimpleIni::IniLine>::iterator itKey=itSection->second.find(key);
	if(itKey == itSection->second.end()) return "";

	return itKey->second.comment;
}

void TSimpleIni::SetComment(const string& section, const string& key, const string& comment)
{
    IniLine iniLine;

	iniLine = m_IniMap[section][key];
	iniLine.comment = comment;
	m_IniMap[section][key] = iniLine;
}

void TSimpleIni::DeleteKey(const string& section, const string& key)
{
    m_IniMap[section].erase(key);
}

TSimpleIni::SectionIterator TSimpleIni::beginSection()
{
    return SectionIterator(m_IniMap.begin());
}

TSimpleIni::SectionIterator TSimpleIni::endSection()
{
    return SectionIterator(m_IniMap.end());
}

TSimpleIni::KeyIterator TSimpleIni::beginKey(const std::string& section)
{
   	map<string, map<string, TSimpleIni::IniLine> >::iterator itSection=m_IniMap.find(section);
	if(itSection == m_IniMap.end())
    {
        itSection = m_IniMap.begin();
        return KeyIterator(itSection->second.end());
    }

    return KeyIterator(itSection->second.begin());
}

TSimpleIni::KeyIterator TSimpleIni::endKey(const std::string& section)
{
   	map<string, map<string, TSimpleIni::IniLine> >::iterator itSection=m_IniMap.find(section);
	if(itSection == m_IniMap.end()) itSection = m_IniMap.begin();

    return KeyIterator(itSection->second.end());
}

void TSimpleIni::ParasitCar(string& str)
{
    size_t fin=str.size();

    if(fin<1) return;

    if(str.at(fin-1)<' ') str.erase(fin-1);
}

string TSimpleIni::Trim(const string& str)
{
    size_t deb=0;
    size_t fin=str.size();
    char   chr;

    while(deb<fin)
    {
        chr = str.at(deb);
        if( (chr!=' ') && (chr!='\t') ) break;
        deb++;
    }

    while(fin>0)
    {
        chr = str.at(fin-1);
        if( (chr!=' ') && (chr!='\t') ) break;
        fin--;
    }

    return str.substr(deb, fin-deb);
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class SectionIterator                                                                                  ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
TSimpleIni::SectionIterator::SectionIterator()
{
}

TSimpleIni::SectionIterator::SectionIterator(std::map<std::string, std::map<std::string, TSimpleIni::IniLine> >::iterator mapIterator)
{
    m_mapIterator = mapIterator;
}

const std::string& TSimpleIni::SectionIterator::operator*()
{
    return m_mapIterator->first;
}

TSimpleIni::SectionIterator TSimpleIni::SectionIterator::operator++()
{
    ++m_mapIterator;
    return *this;
}

bool TSimpleIni::SectionIterator::operator==(SectionIterator const& a)
{
    return a.m_mapIterator==m_mapIterator;
}

bool TSimpleIni::SectionIterator::operator!=(SectionIterator const& a)
{
    return a.m_mapIterator!=m_mapIterator;
}

/**************************************************************************************************************/
/***                                                                                                        ***/
/*** Class KeyIterator                                                                                      ***/
/***                                                                                                        ***/
/**************************************************************************************************************/
TSimpleIni::KeyIterator::KeyIterator()
{
}

TSimpleIni::KeyIterator::KeyIterator(std::map<std::string, TSimpleIni::IniLine>::iterator mapIterator)
{
    m_mapIterator = mapIterator;
}

const std::string& TSimpleIni::KeyIterator::operator*()
{
    return m_mapIterator->first;
}

TSimpleIni::KeyIterator TSimpleIni::KeyIterator::operator++()
{
    ++m_mapIterator;
    return *this;
}

bool TSimpleIni::KeyIterator::operator==(KeyIterator const& a)
{
    return a.m_mapIterator==m_mapIterator;
}

bool TSimpleIni::KeyIterator::operator!=(KeyIterator const& a)
{
    return a.m_mapIterator!=m_mapIterator;
}

// TESTE USimpleIni

/*#include <iostream>
#include "USimpleIni.h"

using namespace std;

int main()
{
    TSimpleIni ini;
    string host;
    int port;


    ini.Load("test.ini");
    host = ini.GetValue<string>("SGBD", "Host", "127.0.0.1");
    port = ini.GetValue<int>("SGBD", "PortTCP", 3306);
    cout << "SGBD Host : " << host << endl;
    cout << "Port TCP : " << port << endl;
    cout << "Database : " << ini.GetValue<string>("SGBD", "BDD", "MyBDD") << endl;

    ini.SetValue<string>("sectionB", "cleB1", "valeurB1");      //Add section and key with value
    ini.SetValue<float>("sectionB", "cleB2", 0.5);              //Add key with value

    if(!ini.SaveAs("test.ini"))
    {
        cout << "Impossible de sauver example3bis.ini." << endl;
        return -1;
    }

    cout << "Le fichier de configuration : example3bis.ini." << endl;

    return 0;
}*/

