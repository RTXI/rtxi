/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.

         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.

         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <algorithm>

#include "event.hpp"

#include "debug.hpp"

void Event::Object::wait()
{
  std::unique_lock done_lock(this->processing_done_mut);
  this->processing_done_cond.wait(done_lock, [this](){return this->processed;});
  //done_lock.unlock();
}

void Event::Object::done()
{
  std::unique_lock done_lock(this->processing_done_mut);
  this->processed = true;
  //done_lock.unlock();
  this->processing_done_cond.notify_one();
}

bool Event::Object::isdone() const
{
  return this->processed;
}

Event::Manager::~Manager()
{
  this->handlerList.clear();
}

void Event::Manager::postEvent(Event::Object* event)
{
  for (auto & handler : this->handlerList) {
    handler->receiveEvent(event);
  }
}

void Event::Manager::registerHandler(Event::Handler* handler)
{
  auto location = std::find(handlerList.begin(), handlerList.end(), handler);
  if(location == handlerList.end()){
    handlerList.push_back(handler);
  }
}

void Event::Manager::unregisterHandler(Handler* handler)
{
  auto location = std::find(handlerList.begin(), handlerList.end(), handler);
  if(location != handlerList.end()){
    handlerList.erase(location);
  }
}