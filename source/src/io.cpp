/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <compiler.h>
#include <debug.h>
#include <event.h>
#include <io.h>
#include <algorithm>
#include <sstream>
#include <string>

Mutex IO::Block::mutex = Mutex(Mutex::RECURSIVE);

IO::Block::Block(std::string n,IO::channel_t *channel,size_t size)
    : name(n) {
    for(size_t i=0;i<inputs.size();++i)
        while(inputs[i].links.size())
            disconnect(inputs[i].links.front().block,inputs[i].links.front().channel,this,i);
    for(size_t i=0;i<outputs.size();++i)
        while(outputs[i].links.size())
            disconnect(this,i,outputs[i].links.front().block,outputs[i].links.front().channel);

    int num_inputs = 0, num_outputs = 0;
    for(size_t i = 0;i < size;++i)
        if(channel[i].flags & INPUT) num_inputs++;
        else if(channel[i].flags & OUTPUT) num_outputs++;

    inputs = std::vector<struct input_t>(num_inputs);
    outputs = std::vector<struct output_t>(num_outputs);

    size_t in = 0, out = 0;
    for(size_t i = 0;i < size;++i)
        if(channel[i].flags & INPUT) {
            inputs[in].name = channel[i].name;
            inputs[in++].description = channel[i].description;
        } else if(channel[i].flags & OUTPUT) {
            outputs[out].name = channel[i].name;
            outputs[out].description = channel[i].description;
            outputs[out++].value = 0.0;
        }

    IO::Connector::getInstance()->insertBlock(this);
}

IO::Block::~Block(void) {
    IO::Connector::getInstance()->removeBlock(this);

    Mutex::Locker lock(&mutex);
    for(size_t i=0;i<inputs.size();++i)
        while(inputs[i].links.size())
            disconnect(inputs[i].links.front().block,inputs[i].links.front().channel,this,i);
    for(size_t i=0;i<outputs.size();++i)
        while(outputs[i].links.size())
            disconnect(this,i,outputs[i].links.front().block,outputs[i].links.front().channel);

    inputs = std::vector<struct input_t>();
    outputs = std::vector<struct output_t>();
}

size_t IO::Block::getCount(IO::flags_t type) const {
    if(type & INPUT)
        return inputs.size();
    if(type & OUTPUT)
        return outputs.size();
    return 0;
}

std::string IO::Block::getName(IO::flags_t type,size_t n) const {
    if(type & INPUT && n < inputs.size())
        return inputs[n].name;
    if(type & OUTPUT && n < outputs.size())
        return outputs[n].name;
    return "";
}

std::string IO::Block::getDescription(IO::flags_t type,size_t n) const {
    if(type & INPUT && n < inputs.size())
        return inputs[n].description;
    if(type & OUTPUT && n < outputs.size())
        return outputs[n].description;
    return "";
}

double IO::Block::getValue(IO::flags_t type,size_t n) const {
    if(type & INPUT)
        return input(n);
    if(type & OUTPUT)
        return output(n);
    return 0.0;
}

double IO::Block::input(size_t n) const {
    if(unlikely(n >= inputs.size()))
        return 0.0;

    double v = 0.0;
    for(std::list<struct link_t>::const_iterator i = inputs[n].links.begin(),end = inputs[n].links.end();i != end;++i)
        v += i->block->output(i->channel);
    return v;
}

double IO::Block::output(size_t n) const {
    if(unlikely(n >= outputs.size()))
        return 0.0;
    return outputs[n].value;
}

double IO::Block::junk = 0.0;

double &IO::Block::output(size_t n) {

    /*********************************************************
     * This seems kinda sketchy, but we have to return       *
     *   something if the user requests the wrong channel... *
     *********************************************************/

    if(unlikely(n >= outputs.size()))
        return junk = 0.0;
    return outputs[n].value;
}

void IO::Block::connect(IO::Block *src,size_t src_num,IO::Block *dest,size_t dest_num) {
    Mutex::Locker lock(&mutex);

    if(!src) {
        ERROR_MSG("Block::connect : invalid source\n");
        return;
    }
    if(src_num >= src->outputs.size()) {
        ERROR_MSG("Block::connect : invalid source channel\n");
        return;
    }

    if(!dest) {
        ERROR_MSG("Block::connect : invalid destination\n");
        return;
    }
    if(dest_num >= dest->inputs.size()) {
        ERROR_MSG("Block::connect : invalid destination channel\n");
        return;
    }

    // Check if the connection exists
    for(std::list<struct link_t>::const_iterator i=src->outputs[src_num].links.begin();i != src->outputs[src_num].links.end();++i)
        if(i->block == dest && i->channel == dest_num) {
            ERROR_MSG("Block::connect : connection exists\n");
            return;
        }

    struct link_t link;

    // Make the forward connection (src => dest)
    link.block = dest;
    link.channel = dest_num;
    src->outputs[src_num].links.push_back(link);

    // Make the backward connection (src <= dest)
    link.block = src;
    link.channel = src_num;
    dest->inputs[dest_num].links.push_back(link);

    Event::Object event(Event::IO_LINK_INSERT_EVENT);
    event.setParam("src",src);
    event.setParam("src_num",&src_num);
    event.setParam("dest",dest);
    event.setParam("dest_num",&dest_num);
    Event::Manager::getInstance()->postEvent(&event);
}

void IO::Block::disconnect(IO::Block *src,size_t src_num,IO::Block *dest,size_t dest_num) {
    Mutex::Locker lock(&mutex);

    if(!src) {
        ERROR_MSG("Block::disconnect : invalid source\n");
        return;
    }
    if(src_num >= src->outputs.size()) {
        ERROR_MSG("Block::disconnect : invalid source channel\n");
        return;
    }

    if(!dest) {
        ERROR_MSG("Block::disconnect : invalid destination\n");
        return;
    }
    if(dest_num >= dest->inputs.size()) {
        ERROR_MSG("Block::disconnect : invalid destination channel\n");
        return;
    }

    Event::Object event(Event::IO_LINK_REMOVE_EVENT);
    event.setParam("src",src);
    event.setParam("src_num",&src_num);
    event.setParam("dest",dest);
    event.setParam("dest_num",&dest_num);
    Event::Manager::getInstance()->postEvent(&event);

    // Remove the forward connection (src => dest)
  start_forward_remove:
    for(std::list<struct link_t>::iterator i=src->outputs[src_num].links.begin();i != src->outputs[src_num].links.end();++i)
        if(i->block == dest && i->channel == dest_num) {
            src->outputs[src_num].links.erase(i);
            goto start_forward_remove;
        }

    // Remove the backward connection (src <= dest)
  start_backward_remove:
    for(std::list<struct link_t>::iterator i=dest->inputs[dest_num].links.begin();i != dest->inputs[dest_num].links.end();++i)
        if(i->block == src && i->channel == src_num) {
            dest->inputs[dest_num].links.erase(i);
            goto start_backward_remove;
        }
}

void IO::Connector::foreachBlock(void (*callback)(Block *,void *),void *param) {
    Mutex::Locker lock(&mutex);
    for(std::list<Block *>::iterator i = blockList.begin();i != blockList.end();++i)
        callback(*i,param);
}

void IO::Connector::foreachConnection(void (*callback)(Block *,size_t,Block *,size_t,void *),void *param) {
    Mutex::Locker lock(&mutex);

    for(std::list<Block *>::iterator i = blockList.begin(), iend = blockList.end();i != iend;++i)
        for(size_t j = 0, jend = (*i)->outputs.size();j < jend;++j)
            for(std::list<Block::link_t>::iterator k = (*i)->outputs[j].links.begin(), kend = (*i)->outputs[j].links.end();k != kend;++k)
                callback(*i,j,k->block,k->channel,param);
}

void IO::Connector::connect(IO::Block *src,size_t out,IO::Block *dest,size_t in) {
    Mutex::Locker lock(&mutex);

    if(!src) {
        ERROR_MSG("IO::Connector::connect : invalid output block\n");
        return;
    }

    if(!dest) {
        ERROR_MSG("IO::Connector::connect : invalid input block\n");
        return;
    }

    IO::Block::connect(src,out,dest,in);
}

void IO::Connector::disconnect(IO::Block *src,size_t out,IO::Block *dest,size_t in) {
    Mutex::Locker lock(&mutex);

    if(!src) {
        ERROR_MSG("IO::Connector::disconnect : invalid output block\n");
        return;
    }

    if(!dest) {
        ERROR_MSG("IO::Connector::disconnect : invalid input block\n");
        return;
    }

    IO::Block::disconnect(src,out,dest,in);
}

bool IO::Connector::connected(IO::Block *src,size_t src_num,IO::Block *dest,size_t dest_num) {
    if(!src) {
        ERROR_MSG("Block::connected : invalid source\n");
        return false;
    }
    if(src_num >= src->outputs.size()) {
        ERROR_MSG("Block::connected : invalid source channel\n");
        return false;
    }

    if(!dest) {
        ERROR_MSG("Block::connected : invalid destination\n");
        return false;
    }
    if(dest_num >= dest->inputs.size()) {
        ERROR_MSG("Block::connected : invalid destination channel\n");
        return false;
    }

    for(std::list<IO::Block::link_t>::iterator i=src->outputs[src_num].links.begin();i != src->outputs[src_num].links.end();++i)
        if(i->block == dest && i->channel == dest_num)
            return true;

    return false;
}

void IO::Connector::doDeferred(const Settings::Object::State &s) {
    Block *src, *dest;

    for(size_t i = 0,end = s.loadInteger("Num Links");i < end;++i) {
        std::ostringstream str;
        str << i;

        src = dynamic_cast<Block *>(Settings::Manager::getInstance()->getObject(s.loadInteger(str.str()+" Source ID")));
        dest = dynamic_cast<Block *>(Settings::Manager::getInstance()->getObject(s.loadInteger(str.str()+" Destination ID")));
        if(src && dest)
            connect(src,s.loadInteger(str.str()+" Source channel"),
                    dest,s.loadInteger(str.str()+" Destination channel"));
    }
}

void IO::Connector::doSave(Settings::Object::State &s) const {
    size_t n = 0;

    for(std::list<Block *>::const_iterator i = blockList.begin(),iend = blockList.end();i != iend;++i)
        for(size_t j = 0,jend = (*i)->getCount(INPUT);j < jend;++j)
            for(std::list<Block::link_t>::const_iterator k = (*i)->inputs[j].links.begin(),kend = (*i)->inputs[j].links.end();k != kend;++k) {
                std::ostringstream str;
                str << n++;
                s.saveInteger(str.str()+" Source ID",k->block->getID());
                s.saveInteger(str.str()+" Source channel",k->channel);
                s.saveInteger(str.str()+" Destination ID",(*i)->getID());
                s.saveInteger(str.str()+" Destination channel",j);
            }
    s.saveInteger("Num Links",n);
}

void IO::Connector::insertBlock(IO::Block *block) {
    if(!block) {
        ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
        return;
    }

    Mutex::Locker lock(&mutex);

    if(std::find(blockList.begin(),blockList.end(),block) != blockList.end()) {
        ERROR_MSG("IO::Connector::insertBlock : block already present\n");
        return;
    }

    Event::Object event(Event::IO_BLOCK_INSERT_EVENT);
    event.setParam("block",block);
    Event::Manager::getInstance()->postEvent(&event);

    blockList.push_back(block);
}

void IO::Connector::removeBlock(IO::Block *block) {
    if(!block) {
        ERROR_MSG("IO::Connector::insertBlock : invalid block\n");
        return;
    }

    Mutex::Locker lock(&mutex);

    Event::Object event(Event::IO_BLOCK_REMOVE_EVENT);
    event.setParam("block",block);
    Event::Manager::getInstance()->postEvent(&event);

    blockList.remove(block);
}

static Mutex mutex;
IO::Connector *IO::Connector::instance = 0;

IO::Connector *IO::Connector::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static Connector connector;
        instance = &connector;
    }

    return instance;
}
