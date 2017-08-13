//
//  ShaderMgr.cpp
//  SimpleBall
//
//  Created by Patrick  Wingo on 9/1/13.
//  Copyright (c) 2013 Patrick  Wingo. All rights reserved.
//

#include "ShaderMgr.h"
#include <iostream>

//extern const char* VERT_EXT;
//extern const char* FRAG_EXT;
//extern const char* GEOM_EXT;

const char* VERT_EXT=".vert";
const char* FRAG_EXT=".frag";
const char* GEOM_EXT=".geom";
const char* COMP_EXT=".comp";
const char* TCS_EXT=".tcs";
const char* TES_EXT=".tes";

enum {V_SHAD,F_SHAD,G_SHAD,TE_SHAD,TC_SHAD,C_SHAD,SHADCOUNT};

/** \addtogroup fileExts Supported Shader file extensions
    @{
        These are the expected shader extensions for use with this class.
        - __.vert__ - Vertex Shader.
        - __.frag__ - Fragment Shader.
        - __.geom__ - Geometry Shader.
        - __.comp__ - Compute Shader (if supported).
        - __.teev__ - Tessellation Evaluation Shader.
        - __.tect__ - Tessellation Control Shader.
    @}
 */

/** Constructor.
    @param relative Optional. Absolute path to use to resolve relative paths.
 */
ShaderMgr::ShaderMgr(STLString relative)
:m_relDir(relative)
{
    if(!m_relDir.empty() && m_relDir.back()!='/')
        m_relDir+='/';
}

/** Destructor */
ShaderMgr::~ShaderMgr()
{
    DeleteAllPrograms();
}

/** Retrieve index for shader program by user-designated name.
    @param name The name of the program to query for.
    @return The index of the named program, or 0 if the program is not found.
 */
GLuint ShaderMgr::ProgForName(const STLString & name)
{
    GLuint ret=0;
    if(m_progs.find(name)!=m_progs.end())
        ret=m_progs[name];
    
    return ret;
}

/** Load a "set" of shaders; one or more shaders will be loaded with name based on their distinct extensions as they are found.
 
    @param name The common name for the shaders to include.
    @param useLazy If true, the shader are not compiled and loaded until the first time they are requested. Otherwise, they are loaded and compiled together immediately.
    @return The index of the shader program if compilation and linking was successful; 0 is returned if the programs are set to lazy load, or if compilation and linking was unsuccessful.
    @sa fileExts
 */
GLuint ShaderMgr::LoadShaderProgramSet(const STLString & name,const bool & useLazy)
{
    STLString vertPath=name+VERT_EXT;
    STLString fragPath=name+FRAG_EXT;
    STLString geomPath=name+GEOM_EXT;
    STLString compPath=name+COMP_EXT;
    STLString teevPath=name+TES_EXT;
    STLString tectPath=name+TCS_EXT;
    
    if(!FileExists((m_relDir+vertPath).c_str()))
        vertPath.clear();
    if(!FileExists((m_relDir+fragPath).c_str()))
        fragPath.clear();
    if(!FileExists((m_relDir+geomPath).c_str()))
        geomPath.clear();
    if(!FileExists((m_relDir+compPath).c_str()))
        compPath.clear();
    if(!FileExists((m_relDir+teevPath).c_str()))
        teevPath.clear();
    if(!FileExists((m_relDir+tectPath).c_str()))
        tectPath.clear();

    //remove prefix from path
    if(useLazy)
    {
        LazyLoads entry;
        entry.vert=vertPath;
        entry.frag=fragPath;
        entry.geom=geomPath;
        entry.comp=compPath;
        entry.teev=teevPath;
        entry.tect=tectPath;
        m_lazys[name]=entry;
    }
    else
        return LoadShaderProgram(name,vertPath,fragPath,geomPath,teevPath,tectPath,compPath);
    return 0;
}

/** Load, Compile, and Link a shader program, and save it under a specified name.
    @param name The name to apply to the shader program within the internal catalog. Required.
    @param vertFile An absolute path to a vertex shader file. Optional.
    @param fragFile An absolute path to a fragment shader file. Optional.
    @param geomFile An absolute path to a geometry shader file. Optional.
    @param teevFile An absolute path to a tessellation evaluation shader file. Optional.
    @param tectFile An absolute path to a tessellation control shader file. Optional.
    @param compFile An absolute path to a compute shader file. Optional.
 
    @return The index of the program, if the program generation process was successful; otherwise, 0 is returned on failure.
 
 */
GLuint ShaderMgr::LoadShaderProgram(const STLString & name, const STLString & vertFile, const STLString & fragFile, const STLString & geomFile, const STLString & teevFile, const STLString & tectFile,const STLString & compFile)
{
    
    GLuint shaders[SHADCOUNT];
    for(size_t i=0; i<SHADCOUNT; i++)
        shaders[i]=0;
    
	GLuint ret=0;
    bool go=true;
    if(!vertFile.empty())
    {
        std::cout<<"Loading VertShader "<<vertFile<<std::endl;
        shaders[V_SHAD]=ConstructShader(vertFile, GL_VERTEX_SHADER);
        go=shaders[V_SHAD]>0;
    }
    
    if(!fragFile.empty())
    {
        std::cout<<"Loading FragShader "<<fragFile<<std::endl;
        shaders[F_SHAD]=ConstructShader(fragFile, GL_FRAGMENT_SHADER);
        go=shaders[F_SHAD]>0;
    }

    if(!geomFile.empty())
    {
        std::cout<<"Loading GeomShader "<<geomFile<<std::endl;
        shaders[G_SHAD]=ConstructShader(geomFile, GL_GEOMETRY_SHADER);
        go=shaders[G_SHAD]>0;
    }

    if(!tectFile.empty())
    {
        std::cout<<"Loading Tessellation Control Shader "<<tectFile<<std::endl;
        if(!teevFile.empty())
        {
            shaders[TC_SHAD]=ConstructShader(tectFile, GL_TESS_CONTROL_SHADER);
            go=shaders[TC_SHAD]>0;
        }
        else
            std::cout<<"Err! no Tessellation Eval Shader found! Skipping "<<tectFile<<std::endl;
    }
    if(!teevFile.empty())
    {
        std::cout<<"Loading Tessellation Eval Shader "<<teevFile<<std::endl;
        if(!tectFile.empty())
        {
            shaders[TE_SHAD]=ConstructShader(teevFile, GL_TESS_EVALUATION_SHADER);
            go=shaders[TE_SHAD]>0;
        }
        else
            std::cout<<"Err! no Tessellation Control Shader found! Skipping "<<teevFile<<std::endl;
    }

    //enable if/when OS X supports compute shaders
//    if(!compFile.empty())
//    {
//        std::cout<<"Loading Compute Shader "<<compFile<<std::endl;
//        shaders[C_SHAD]=ConstructShader(fragFile, GL_COMPUTE_SHADER);
//        go=shaders[C_SHAD]>0;
//    }

    if(go)
    {
        ret=glCreateProgram();
        
        for(size_t i=0; i<SHADCOUNT; i++)
        {
            if(shaders[i])
                glAttachShader(ret, shaders[i]);
        }
        
        GLint isLinked;
        glLinkProgram(ret);
        glGetProgramiv(ret, GL_LINK_STATUS, &isLinked);
        if(!isLinked)
        {
            std::cerr<<"Failed to link shader."<<std::endl;
            
            GLint maxLength;
            glGetProgramiv(ret, GL_INFO_LOG_LENGTH, &maxLength);
            if(maxLength>0)
            {
                char *pLinkInfoLog = new char[maxLength];
                glGetProgramInfoLog(ret, maxLength, &maxLength, pLinkInfoLog);
                std::cerr<<pLinkInfoLog<<std::endl;
                delete [] pLinkInfoLog;
            }
            
            for(size_t i=0; i<SHADCOUNT; i++)
            {
                if(shaders[i])
                {
                    glDetachShader(ret, shaders[1]);
                    shaders[i]=0;
                }
            }
            
            glDeleteProgram(ret);
            ret=0;
            
            
        }
        
        
        //DumpInfoLog(ret, true);
        m_progs[name]=ret;
        
    }
    std::cout<<"Shaders loaded"<<std::endl;

    return ret;
}

/** Retrieve a program based on the catalog name and ask OpenGL to activate it. If the shader program is marked as lazy and hasn't been loaded, then the program is compiled and linked before it is activated. If no program with the name of prog is found, then the current shader program will be disabled.
 
    @param prog The name of the program to search for.
    @return The index of the activated program, or 0 if the program was not found.
 */
GLuint ShaderMgr::ActivateProgram(const STLString & prog)
{
    GLuint use=0;
    
    if(m_progs.find(prog)!=m_progs.end())
        use=m_progs[prog];
    else if(m_lazys.find(prog)!=m_lazys.end())
        use=LoadShaderProgram(prog,m_lazys[prog].vert,m_lazys[prog].frag,m_lazys[prog].geom,m_lazys[prog].teev,m_lazys[prog].tect,m_lazys[prog].comp);

    glUseProgram(use);
    return use;
}

/**
 *  Perform validation step on specific shaders, and dump log on failure.
 *  This will fail if there is nothing loaded in the OpenGL pipeline.
 *
 *  @param name The name of the shader program to validate
 *
 *  @return true if validation succeeds, false otherwise.
 */
bool ShaderMgr::ValidateProgram(const STLString & name)
{
    GLuint prog=ActivateProgram(name);
    glValidateProgram(prog);
    GLint validStatus;
    bool ret=false;
    
    if(prog!=0)
    {
        glGetProgramiv(prog, GL_VALIDATE_STATUS, &validStatus);
        if(validStatus)
            ret=true;
        else
        {
            printf("Failed to validate shader.");
            
            DumpInfoLog(prog, false);
        }
    
    }
    return ret;
}

/** Delete a specific program from the OpenGL pipeline by name.
    @param prog The name of the program.
 */
void ShaderMgr::DeleteProgram(const STLString & prog)
{
    if(m_progs.find(prog)!=m_progs.end())
        glDeleteProgram(m_progs[prog]);
}

/** Delete all programs from the GL engine. */
void ShaderMgr::DeleteAllPrograms()
{
    for(auto itr=m_progs.begin(); itr!=m_progs.end(); ++itr)
        glDeleteProgram((*itr).second);
}

/** Disables the currently active shader program.
 */
void ShaderMgr::DisableShaders()
{
    glUseProgram(0);
}

/** Loads and Compiles a shader for use with a shader program.
    @param filepath and absolute path to the shader file to load.
    @param shader the type of shader to compile.
    @return 1 on successful compilation; 0 otherwise.
 */
int ShaderMgr::CompileFile(const STLString & filepath,const GLuint shader)
{
    STLString fullPath=filepath;
    
    if(!m_relDir.empty() && filepath[0]!='/')
        fullPath=m_relDir+fullPath;
    
	using namespace std;
	ifstream file(fullPath,ifstream::in);
    //	file.open(filepath,ios::in); //ASCII
	if(!file || !file.good())
		return 0;
	
	//get length
	unsigned long pos=file.tellg();
	file.seekg(0,ios::end);
	unsigned long len=file.tellg();
	file.seekg(pos);
	//
	
	if(!len)
		return 0;
	
	GLchar** source=new GLchar*;
	GLchar* src=new GLchar[len+1];
	
	*source=src;
	
	if (!src)
		return 0;
	
	//ensure 0 term
	src[len]=0;
	
	//load by character
	GLuint i=0;
	while(file.good())
	{
		src[i] = file.get();
        //		cout<<src[i];
		if(!file.eof())
			i++;
	}
	
	src[i]=0; //0 term real end
	
	GLint* lineCounts=new GLint[1];
	lineCounts[0]=i;
	
	file.close();
	
	glShaderSource(shader, 1, (const GLchar**)source, lineCounts);
	glCompileShader(shader);
    GLint params;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
    if(params==GL_FALSE)
        return 0;
    
#ifdef DUMP_SHADERS
    cout<<src<<endl<<endl;
#endif
    return 1;
    
}

/** Construct a shader object and compile the code at path.
    @param path An absolute file path to a shader to compile.
    @param type The type of shader to compile.
    @return The index of the shader if compilation was successful; 0 otherwise.
 */
GLuint ShaderMgr::ConstructShader(const STLString & path, const GLuint type)
{
    GLuint shader=0;
    
    shader=glCreateShader(type);
    
    int status;
    bool go=true;
    if(!shader)
    {
        std::cerr<<"Could not create Shader"<<std::endl;
        go=false;
    }
    
    if(!CompileFile(path, shader))
    {
        std::cerr<<"Failed to load file"<<std::endl;
        DumpInfoLog(shader, true);

        go=false;
    }

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
        std::cerr<<"Shader Compile fail"<<std::endl;
        go=false;
	}
    
    
    if(!go)
        shader=0;
    return shader;
}

/** Print the shader or program log generated by the OpenGL drivers to the cerr stream.
    @param prog Index of the program or shader to retrieve information for.
    @param forShader If true, applies the index prog to the shader logs; otherwise prog is applied to the program logs.
 */
void ShaderMgr::DumpInfoLog(const GLuint prog,const bool forShader)
{
    GLint maxLength;
    forShader ? glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &maxLength) : glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxLength);
    if(maxLength>0)
    {
        char *pInfoLog = new char[maxLength];
        forShader ? glGetShaderInfoLog(prog, maxLength, NULL, pInfoLog) : glGetProgramInfoLog(prog, maxLength, &maxLength, pInfoLog);
        std::cerr<<pInfoLog<<std::endl;
        delete [] pInfoLog;
    }
}
