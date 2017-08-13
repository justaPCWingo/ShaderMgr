//
//  ShaderMgr.h
//  SimpleBall
//
//  Created by Patrick  Wingo on 9/1/13.
//  Copyright (c) 2013 Patrick  Wingo. All rights reserved.
//

#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <OpenGL/gl3.h>

#define ADDROFFSET(x) (char*)NULL+(sizeof(float)*(x))

typedef std::string STLString;
typedef std::map<STLString,GLuint> ProgMap;
typedef std::map<STLString,GLuint>::iterator ProgItr;

class ShaderMgr
{
public:
    
    ShaderMgr(STLString relative=STLString());
    
    GLuint LoadShaderProgram(const STLString & name, const STLString & vertFile=STLString(), const STLString & fragFile=STLString(), const STLString & geomFile=STLString(), const STLString & teevFile=STLString(), const STLString & tectFile=STLString(),const STLString & compFile=STLString());
    GLuint LoadShaderProgramSet(const STLString & name,const bool & useLazy=false);

    virtual ~ShaderMgr();
    
    GLuint ProgForName(const STLString & name);
    GLuint ActivateProgram(const STLString & prog=STLString());
    bool ValidateProgram(const STLString & name);
    
    void DeleteProgram(const STLString & prog);
    void DeleteAllPrograms();
    void DisableShaders();
    
    ProgItr ProgBegin() { return m_progs.begin(); }
    ProgItr ProgEnd() { return m_progs.end(); }
    
    inline void SetRelativeDirectory(const STLString & str) { m_relDir=str;}
    inline STLString GetRelativeDirectory() const {return m_relDir;}
    
    
private:
    
    GLuint ConstructShader(const STLString & path, const GLuint type);
    int CompileFile(const STLString & filepath,const GLuint shader);
    void DumpInfoLog(const GLuint prog,const bool forShader);
    
    
    inline bool FileExists(const char *filename)
    {
        std::ifstream ifile(filename);
        return (bool)ifile;
    }

    //TODO: add shader program updater
    ProgMap m_progs;
    STLString m_relDir;
    
    struct LazyLoads
    {
        STLString vert;
        STLString frag;
        STLString geom;
        STLString comp;
        STLString teev;
        STLString tect;
    };
    
    std::map<STLString,LazyLoads> m_lazys;
};

