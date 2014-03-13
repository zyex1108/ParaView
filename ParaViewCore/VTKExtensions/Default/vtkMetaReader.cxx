/*=========================================================================

  Program:   ParaView
  Module:    vtkFileSeriesReader.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkMetaReader.h"

#include "vtkClientServerInterpreterInitializer.h"
#include "vtkClientServerInterpreter.h"
#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"

//=============================================================================
vtkStandardNewMacro(vtkMetaReader);

//----------------------------------------------------------------------------
vtkMetaReader::vtkMetaReader() :
  Reader (NULL), FileNameMTime(0), BeforeFileNameMTime(0),
  FileNameMethod(NULL), _FileIndex(0), FileIndexMTime(0),
  _MetaFileName(NULL), MetaFileNameMTime (0)
{
  this->FileIndexRange[0] = 0;
  this->FileIndexRange[1] = 0;
  this->SetNumberOfInputPorts(0);
}

vtkMetaReader::~vtkMetaReader()
{
  this->SetMetaFileName(NULL);
  if(this->Reader)
    {
    this->Reader->Delete();
    }
  this->SetFileNameMethod(NULL);
}
//----------------------------------------------------------------------------
void vtkMetaReader::ReaderSetFileName(const char* name)
{
  if (this->Reader && this->FileNameMethod)
    {
    vtkClientServerInterpreter *interpreter =
        vtkClientServerInterpreterInitializer::GetGlobalInterpreter();

    // Build stream request
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke
           << this->Reader
           << this->FileNameMethod
           << name
           << vtkClientServerStream::End;

    // Process stream and delete interpreter
    interpreter->ProcessStream(stream);
    }
}

//-----------------------------------------------------------------------------
int vtkMetaReader::ReaderCanReadFile(const char *filename)
{
  if(this->Reader)
    {
    int canRead = 1;
    vtkClientServerInterpreter *interpreter =
      vtkClientServerInterpreterInitializer::GetGlobalInterpreter();

    // Build stream request
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke
           << this->Reader
           << "CanReadFile"
           << filename
           << vtkClientServerStream::End;

    // Process stream and get result
    interpreter->ProcessStream(stream);
    interpreter->GetLastResult().GetArgument(0, 0, &canRead);
    return canRead;
    }
  return 0;
}

//----------------------------------------------------------------------------
unsigned long vtkMetaReader::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();

  if ( this->Reader )
    {
    // In general, we want changes in Reader to be reflected in this object's
    // MTime.  However, we will also be making modifications to the Reader (such
    // as changing the filename) that we want to suppress from the reporting.
    // When this happens, we save the timestamp before our modification into
    // this->BeforeFileNameMTime and capture the resulting MTime in
    // this->FileNameMTime.  If we run into that modification,
    // suppress it by reporting the saved modification.
    unsigned long readerMTime;
    if (this->Reader->GetMTime() == this->FileNameMTime)
      {
      readerMTime = this->BeforeFileNameMTime;
      }
    else
      {
      readerMTime = this->Reader->GetMTime();
      }
    mTime = ( readerMTime > mTime ? readerMTime : mTime );
    }

  return mTime;
}

//-----------------------------------------------------------------------------
int vtkMetaReader::FillOutputPortInformation(int port,
                                             vtkInformation* info)
{
  vtkInformation* rinfo = this->Reader->GetOutputPortInformation(port);
  info->CopyEntry(rinfo, vtkDataObject::DATA_TYPE_NAME());
  return 1;
}
