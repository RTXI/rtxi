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
#include <sstream>
#include <string>

#include "io.hpp"

#include "debug.hpp"

IO::Block::Block(std::string n, std::vector<IO::channel_t> channels)
    : name(n)
{
  port_t port = {};
  for (auto channel : channels) {
    port.channel_info = channel;
    port.values = std::vector<double>(channel.data_size, 0.0);
    ports[channel.flags].push_back(port);
    port = {};
  }
}

IO::Block::~Block() {}

size_t IO::Block::getCount(IO::flags_t type) const
{
  return this->ports[type].size();
}

std::string IO::Block::getChannelName(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].channel_info.name;
}

std::string IO::Block::getChannelDescription(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].channel_info.description;
}

std::vector<double> IO::Block::getChannelValue(IO::flags_t type, size_t n) const
{
  return this->ports[type][n].values;
}

void IO::Block::writeinput(size_t index, const std::vector<double>& data)
{
  std::copy(
      data.begin(), data.end(), this->ports[IO::INPUT][index].values.begin());
}

const std::vector<double>& IO::Block::readinput(size_t index)
{
  return this->ports[IO::INPUT][index].values;
}

void IO::Block::writeoutput(size_t index, const std::vector<double>& data)
{
  std::copy(
      data.begin(), data.end(), this->ports[IO::OUTPUT][index].values.begin());
}

const std::vector<double>& IO::Block::readoutput(size_t index)
{
  return this->ports[IO::OUTPUT][index].values;
}

void IO::Connector::foreachBlock(void (*callback)(Block*, void*), void* param)
{
  for (std::list<Block*>::iterator i = blockList.begin(); i != blockList.end();
       ++i)
    callback(*i, param);
}

void IO::Connector::foreachConnection(
    void (*callback)(Block*, size_t, Block*, size_t, void*), void* param)
{

  for (std::list<Block*>::iterator i = blockList.begin(),
                                   iend = blockList.end();
       i != iend;
       ++i)
    for (size_t j = 0, jend = (*i)->outputs.size(); j < jend; ++j)
      for (std::list<Block::link_t>::iterator
               k = (*i)->outputs[j].links.begin(),
               kend = (*i)->outputs[j].links.end();
           k != kend;
           ++k)
        callback(*i, j, k->block, k->channel, param);
}

void IO::Connector::connect(IO::Block* src,
                            size_t out,
                            IO::Block* dest,
                            size_t in)
{
  Mutex::Locker lock(&mutex);

  if (!src) {
    ERROR_MSG("IO::Connector::connect : invalid output block\n");
    return;
  }

  if (!dest) {
    ERROR_MSG("IO::Connector::connect : invalid input block\n");
    return;
  }

  IO::Block::connect(src, out, dest, in);
}

void IO::Connector::disconnect(IO::Block* src,
                               size_t out,
                               IO::Block* dest,
                               size_t in)
{
  Mutex::Locker lock(&mutex);

  if (!src) {
    ERROR_MSG("IO::Connector::disconnect : invalid output block\n");
    return;
  }

  if (!dest) {
    ERROR_MSG("IO::Connector::disconnect : invalid input block\n");
    return;
  }

  IO::Block::disconnect(src, out, dest, in);
}

bool IO::Connector::connected(IO::Block* src,
                              size_t src_num,
                              IO::Block* dest,
                              size_t dest_num)
{
  if (!src) {
    ERROR_MSG("Block::connected : invalid source\n");
    return false;
  }
  if (src_num >= src->outputs.size()) {
    ERROR_MSG("Block::connected : invalid source channel\n");
    return false;
  }

  if (!dest) {
    ERROR_MSG("Block::connected : invalid destination\n");
    return false;
  }
  if (dest_num >= dest->inputs.size()) {
    ERROR_MSG("Block::connected : invalid destination channel\n");
    return false;
  }

  for (std::list<IO::Block::link_t>::iterator i =
           src->outputs[src_num].links.begin();
       i != src->outputs[src_num].links.end();
       ++i)
    if (i->block == dest && i->channel == dest_num)
      return true;

  return false;
}

// void IO::Connector::doDeferred(const Settings::Object::State &s)
//{
//    Block *src, *dest;
//
//    for (size_t i = 0,end = s.loadInteger("Num Links"); i < end; ++i)
//        {
//            std::ostringstream str;
//            str << i;
//
//            src = dynamic_cast<Block
//            *>(Settings::Manager::getInstance()->getObject(s.loadInteger(str.str()+"
//            Source ID"))); dest = dynamic_cast<Block
//            *>(Settings::Manager::getInstance()->getObject(s.loadInteger(str.str()+"
//            Destination ID"))); if (src && dest)
//                connect(src,s.loadInteger(str.str()+" Source channel"),
//                        dest,s.loadInteger(str.str()+" Destination channel"));
//        }
//}
//
// void IO::Connector::doSave(Settings::Object::State &s) const
//{
//    size_t n = 0;
//
//    for (std::list<Block *>::const_iterator i = blockList.begin(),iend =
//    blockList.end(); i != iend; ++i)
//        for (size_t j = 0,jend = (*i)->getCount(INPUT); j < jend; ++j)
//            for (std::list<Block::link_t>::const_iterator k =
//            (*i)->inputs[j].links.begin(),kend = (*i)->inputs[j].links.end();
//            k != kend; ++k)
//                {
//                    std::ostringstream str;
//                    str << n++;
//                    s.saveInteger(str.str()+" Source ID",k->block->getID());
//                    s.saveInteger(str.str()+" Source channel",k->channel);
//                    s.saveInteger(str.str()+" Destination ID",(*i)->getID());
//                    s.saveInteger(str.str()+" Destination channel",j);
//                }
//    s.saveInteger("Num Links",n);
//}

void IO::Connector::insertBlock(IO::Block* block)
{
  if (!block) {
    ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
    return;
  }

  Mutex::Locker lock(&mutex);

  if (std::find(blockList.begin(), blockList.end(), block) != blockList.end()) {
    ERROR_MSG("IO::Connector::insertBlock : block already present\n");
    return;
  }

  Event::Object event(Event::IO_BLOCK_INSERT_EVENT);
  event.setParam("block", block);
  Event::Manager::getInstance()->postEvent(&event);

  blockList.push_back(block);
}

void IO::Connector::removeBlock(IO::Block* block)
{
  if (!block) {
    ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
    return;
  }

  Mutex::Locker lock(&mutex);

  Event::Object event(Event::IO_BLOCK_REMOVE_EVENT);
  event.setParam("block", block);
  Event::Manager::getInstance()->postEvent(&event);

  blockList.remove(block);
}

static Mutex mutex;
IO::Connector* IO::Connector::instance = 0;

IO::Connector* IO::Connector::getInstance(void)
{
  if (instance)
    return instance;

  /*************************************************************************
   * Seems like alot of hoops to jump through, but static allocation isn't *
   *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
   *************************************************************************/

  Mutex::Locker lock(&::mutex);
  if (!instance) {
    static Connector connector;
    instance = &connector;
  }

  return instance;
}
