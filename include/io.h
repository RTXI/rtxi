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

#ifndef IO_H
#define IO_H

#include <list>
#include <mutex.h>
#include <settings.h>
#include <string>
#include <vector>

//! Connection Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 *   for managing data sharing between various entities.
 */
namespace IO {

    /*!
     * Variable used to specify the type of a channel.
     */
    typedef unsigned long flags_t;

    /*!
     * Bitmask to represent an input type channel.
     */
    static const flags_t INPUT  = 0x1;
    /*!
     * Bitmask to represent an output type channel.
     */
    static const flags_t OUTPUT = 0x2;

    /*!
     * Structure used to pass information to an IO::Block upon creation.
     *
     * \sa IO::Block::Block()
     */
    typedef struct {
        std::string name;
        std::string description;
        flags_t flags;
    } channel_t;

    class Block;

    /*!
     * Acts as a central meeting point between Blocks. Provides
     *   interfaces for finding and connecting blocks.
     *
     * \sa IO::Block
     */
    class Connector : public virtual Settings::Object {

        friend class Block;

    public:

        /*!
         * Connector is a Singleton, which means that there can only be one instance.
         *   This function returns a pointer to that single instance.
         *
         * \return The instance of Connector.
         */
        static Connector *getInstance(void);

        /*!
         * Loop through each Block and execute a callback.
         * The callback takes two parameters, a Block pointer and param,
         *   the second parameter to foreachBlock.
         *
         * \param callback The callback function.
         * \param param A parameter to the callback function.
         *
         * \sa IO::Block
         */
        void foreachBlock(void (*callback)(Block *,void *),void *param);

        /*!
         * Loop through each Connection and execute callback.
         * The callback takes 5 parameters:
         *   The source block
         *   The source output.
         *   The destination block.
         *   The destination input.
         *   The last parameter to foreachConnection.
         *
         * \param callback The callback function.
         * \param param A parameter to the callback function.
         *
         * \sa IO::Block
         */
        void foreachConnection(void (*callback)(Block *,size_t,Block *,size_t,void *),void *param);

        /*!
         * Create a connection between the two specified Blocks.
         *
         * \param outputBlock The source of the data.
         * \param outputChannel The source channel of the data.
         * \param inputBlock The destination of the data.
         * \param inputChannel The destination channel of the data.
         *
         * \sa IO::Block
         * \sa IO::Block::input()
         * \sa IO::Block::output()
         */
        void connect(IO::Block *outputBlock,size_t outputChannel,IO::Block *inputBlock,size_t inputChannel);
        /*!
         * Break a connection between the two specified Blocks.
         *
         * \param outputBlock The source of the data.
         * \param outputChannel The source channel of the data.
         * \param inputBlock The destination of the data.
         * \param inputChannel The destination channel of the data.
         *
         * \sa IO::Block
         * \sa IO::Block::input()
         * \sa IO::Block::output()
         */
        void disconnect(IO::Block *outputBlock,size_t outputChannel,IO::Block *inputBlock,size_t inputChannel);

        /*!
         * Determine whether two channels are connected or not.
         *
         * \param outputBlock The source of the data.
         * \param outputChannel THe source channel of the data.
         * \param inputBlock The destination of the data.
         * \param inputChannel The destination channel of the data.
         *
         * \sa IO::Block::connect()
         * \sa IO::Block::disconnect()
         */
        bool connected(IO::Block *outputBlock,size_t outputChannel,IO::Block *inputBlock,size_t inputChannel);

    private:

        void doDeferred(const Settings::Object::State &);
        void doSave(Settings::Object::State &) const;

        /*****************************************************************
         * The constructor, destructor, and assignment operator are made *
         *   private to control instantiation of the class.              *
         *****************************************************************/

        Connector(void) : mutex(Mutex::RECURSIVE) {};
        ~Connector(void) {};
        Connector(const Connector &) {};
        Connector &operator=(const Connector &) { return *getInstance(); }; 

        static Connector *instance;

        void insertBlock(Block *);
        void removeBlock(Block *);

        Mutex mutex;
        std::list<Block *> blockList;

    }; // class Connector

    /*!
     * An object that provides an interface for transparently manipulating external data.
     *
     * \sa Settings::Object
     */
    class Block : public virtual Settings::Object {

        friend class Connector;

    public:

        /*!
         * The constructor needs to be provided with a specification of the channels that
         *   will be embedded in this block in the channels parameter. Fields that are not
         *   of type INPUT or OUTPUT will be safely ignored. Size should be the number of
         *   total fields in the channels parameter, regardless of type.
         *
         * \param name The name of the block.
         * \param channels The channel specification for this block.
         * \param size The number of channels in the specification.
         *
         * \sa IO::channel_t
         */
        Block(std::string name,channel_t *channels,size_t size);
        virtual ~Block(void);

        /*!
         * Get the name of the block.
         *
         * \return Tbe name of the block.
         */
        std::string getName(void) const { return name; };
        /*!
         * Get the number of channels of the specified type.
         *
         * \param type The type of the channels to be counted.
         * \return The number of channels of the specified type.
         */
        virtual size_t getCount(flags_t type) const;
        /*!
         * Get the name of the specified channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The name of the channel.
         */
        virtual std::string getName(flags_t type,size_t index) const;
        /*!
         * Get the description of the specified channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The description of the channel.
         */
        virtual std::string getDescription(flags_t type,size_t index) const;
        /*!
         * Get the value of the specified channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The value of the channel.
         */
        virtual double getValue(flags_t type,size_t index) const;

        /*!
         * Get the value of the specified input channel.
         *
         * \param index The input channel's index.
         * \return The value of the specified input channel.
         */
        double input(size_t index) const;
        /*!
         * Get the value of the specified output channel.
         *
         * \param index The output channel's index.
         * \return The value of the specified output channel.
         *
         * \sa IO::Block::output()
         */
        double output(size_t index) const;

    protected:

        /*!
         * Get a reference to the value of the specified output channel.
         *   This method can be used to set the value of specified output.
         *
         * \param index The output channel's index.
         * \return A reference to the value of the specified output channel.
         *
         * \sa IO::Block::output()
         */
        double &output(size_t index);

    private:

        struct input_t;
        struct output_t;

        /***************************************************************
         * Calls to connect and disconnect are designed such that they *
         *   don't have to synchronize with input() and output(). That *
         *   is to say the lists are always in a readable state. But   *
         *   this means there can only be one writer, so calls to      *
         *   connect() and disconnect() are serialized with mutex.     *
         ***************************************************************/

        static Mutex mutex;
        static void connect(Block *,size_t,Block *,size_t);
        static void disconnect(Block *,size_t,Block *,size_t);

        /*************************************************************
         * junk exists because "double &output(size_t n)" has to     *
         *   return a reference to something if n >= outputs.size(). *
         *************************************************************/

        static double junk;

        struct link_t {
            Block *block;
            size_t channel;
        };

        struct input_t {
            std::string name;
            std::string description;
            std::list<struct link_t> links;
        };

        struct output_t {
            std::string name;
            std::string description;
            double value;
            std::list<struct link_t> links;
        };

        std::string name;
        std::vector<struct input_t> inputs;
        std::vector<struct output_t> outputs;

    }; // class Block


} // namespace IO

#endif // IO_H
