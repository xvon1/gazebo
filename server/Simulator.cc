/*
 *  Gazebo - Outdoor Multi-Robot Simulator
 *  Copyright (C) 2003  
 *     Nate Koenig & Andrew Howard
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* Desc: The Simulator; Top level managing object
 * Author: Jordi Polo
 * Date: 3 Jan 2008
 */

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
//#include <boost/signals.hpp>
//#include <boost/bind.hpp>

#include "World.hh"
#include "Gui.hh"
#include "XMLConfig.hh"
#include "Global.hh"
#include "gazebo.h"
#include "PhysicsEngine.hh"
#include "OgreAdaptor.hh"
#include "OgreCreator.hh"
#include "GazeboMessage.hh"

#include "Simulator.hh"

using namespace gazebo;

Simulator::Simulator()
{
  this->gui=NULL;

  this->pause = false;

  this->pauseTime = 0.0;
  this->startTime = 0.0;
  this->simTime = 0.0;

  this->userQuit = false;

  this->xmlFile=NULL;
}

Simulator::~Simulator()
{
  GZ_DELETE (this->gui)
  GZ_DELETE (this->xmlFile)
}

void Simulator::Load(const std::string &worldFileName, int serverId )
{

  // Load the world file
  this->xmlFile=new gazebo::XMLConfig();

  if (xmlFile->Load(worldFileName) != 0)
  {
    gzthrow ("The XML config file can not be loaded, please make sure is a correct file");
  }
  
  XMLConfigNode *rootNode(xmlFile->GetRootNode());

  // Load the messaging system
  gazebo::GazeboMessage::Instance()->Load(rootNode);

  //Create and initialize the Gui
  this->LoadGui(rootNode);

  //Initialize RenderingEngine
  try
  {
    gazebo::OgreAdaptor::Instance()->Init(rootNode);
  }
  catch (gazebo::GazeboError e)
  {
    std::ostringstream stream;
    stream << "Failed to Initialize the OGRE Rendering system\n" 
              << e << "\n";
    gzthrow(stream.str());
  }

  //Preload basic shapes that can be used anywhere
  gazebo::OgreCreator::CreateBasicShapes();

  //Create the world
  gazebo::World::Instance()->Load(rootNode, serverId);

}

void Simulator::Save(const std::string& filename)
{
  // Saving in the preferred order
  XMLConfigNode* root=xmlFile->GetRootNode();
  gazebo::GazeboMessage::Instance()->Save(root);
  World::Instance()->GetPhysicsEngine()->Save(root);
  this->SaveGui(root);
  gazebo::OgreAdaptor::Instance()->Save(root);
  World::Instance()->Save(root);

  if (xmlFile->Save(filename)<0)
  {
    std::ostringstream stream;
    stream << "The XML file coult not be written back to " << filename << std::endl;
    gzthrow(stream.str());
  }
}


int Simulator::Init()
{
  this->startTime = this->GetWallTime();

  //Initialize the world
  if (gazebo::World::Instance()->Init() != 0)
    return -1;

}

int Simulator::Fini( )
{
  gazebo::World::Instance()->Fini();
  return 0;
}

void Simulator::Update()
{
  double step= World::Instance()->GetPhysicsEngine()->GetStepTime();

  this->simTime += step;

  // Update the physics engine
  if (!Global::GetUserPause() && !Global::GetUserStep() ||
      (Global::GetUserStep() && Global::GetUserStepInc()))
  {
    Global::IncIterations();
    pause=false;
    Global::SetUserStepInc(!Global::GetUserStepInc());
  }
  else
  {
    this->pauseTime += step; 
    pause=true;
  }

}

void Simulator::MainLoop()
{
  while (!this->userQuit)
  {
    this->Update();  //global simulation
    World::Instance()->Update(); //physics
    gazebo::OgreAdaptor::Instance()->Render(); //rendering
    this->gui->Update(); //GUI
  }
}

Gui *Simulator::GetUI()
{
  return this->gui;
}

bool Simulator::isPaused() const
{
  return this->pause;
}

////////////////////////////////////////////////////////////////////////////////
// Get the simulation time
double Simulator::GetSimTime() const
{
  return this->simTime;
}

////////////////////////////////////////////////////////////////////////////////
// Get the pause time
double Simulator::GetPauseTime() const
{
  return this->pauseTime;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the start time
double Simulator::GetStartTime() const
{
  return this->startTime;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the real time (elapsed time)
double Simulator::GetRealTime() const
{
  return this->GetWallTime() - this->startTime;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the wall clock time
double Simulator::GetWallTime() const
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}


void Simulator::SetUserQuit()
{
//  this->Save("test.xml");
  this->userQuit = true;
}



//TODO: Move to Gui or create GuiEngine and move it there
void Simulator::LoadGui(XMLConfigNode *rootNode)
{
  gazebo::XMLConfigNode *childNode;

  childNode = rootNode->GetChild("gui");

  if (childNode)
  {
    int width = childNode->GetTupleInt("size",0,640);
    int height = childNode->GetTupleInt("size",1,480);
    int x = childNode->GetTupleInt("pos",0,0);
    int y = childNode->GetTupleInt("pos",1,0);
    std::string type = childNode->GetString("type","fltk",1);

    gzmsg(1) << "Creating GUI:\n\tType[" << type << "] Pos[" << x << " " << y << "] Size[" << width << " " << height << "]\n";

    // Create the GUI
    this->gui = new gazebo::Gui(x, y, width, height, type+"::Gazebo");

    // Initialize the GUI
    this->gui->Init();

//    gazebo::GuiFinished.connect( boost::bind(&gazebo::Simulator::SetUserQuit,this));
   
  }
  else
  {
    gzthrow("XML file must contain a <rendering:gui> section\n");
  }
}


 void Simulator::SaveGui(XMLConfigNode *node)
 {
  Vector2<int> size;
  XMLConfigNode* childNode = node->GetChild("gui");
  if (childNode)
  {
    size.x = this->gui->GetWidth();
    size.y = this->gui->GetHeight();
    childNode->SetValue("size", size);
    //TODO: node->SetValue("pos", Vector2<int>(x,y));
  }
  
 }
