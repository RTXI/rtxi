/*
 * Copyright (C) 2005 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <io.h>

//! Internal Control Oriented Classes
/*!
 * Objects contained within this namespace are responsible for providing 
 *   standardized interface for the manipulating of IO::Block internals.
 */
namespace Workspace {

    /*!
     * Bitmask used to represent an input type variable.
     *
     * \sa IO::INPUT
     */
    static const IO::flags_t INPUT     = IO::INPUT;
    /*!
     * Bitmask used to represent an output type variable.
     *
     * \sa IO::OUTPUT
     */
    static const IO::flags_t OUTPUT    = IO::OUTPUT;
    /*!
     * Bitmask used to represent a parameter type variable.
     */
    static const IO::flags_t PARAMETER = IO::OUTPUT<<1;
    /*!
     * Bitmask used to represent a state type variable.
     */
    static const IO::flags_t STATE     = IO::OUTPUT<<2;

    /*!
     * Bitmask used to represent an event variable.
     */
    static const IO::flags_t EVENT     = IO::OUTPUT<<3;

    /*!
     * Structure used to pass informatino to a Workspace::Instance upon creation.
     *
     * \sa Workspace::Instance::Instance()
     * \sa IO::channel_t
     */
    typedef IO::channel_t variable_t;

    /*!
     * An object that provides a standardized interface for accessing and
     *   manipulating both internal and external data.
     *
     * \sa IO::Block
     */
    class Instance : public IO::Block {

        friend class Manager;

    public:

        /*!
         * The constructor needs to be provided with a specification of the variables
         *   that will be embedded in this workspace in the variables parameter.
         *   Fields that are not of type INPUT, OUTPUT, PARAMETER, or STATE will be
         *   safely ignored. Size should be the number of total fields in the variables
         *   parameter, regardless of type.
         *
         * \param name The name of the workspace.
         * \param variables The variable specification for this workspace.
         * \param size The number of variables in the specification.
         *
         * \sa Workspace::variable_t
         */
        Instance(std::string name,variable_t *variables,size_t size);
        virtual ~Instance(void);

        /*!
         * Get the number of variables of the specified type.
         *
         * \param type The type of variable to be probed.
         * \return The number of variables of the specified type.
         *
         * \sa IO::Block::getCount()
         */
        size_t getCount(IO::flags_t type) const;
        /*!
         * Get the name of the specified variable.
         *
         * \param type The type of the variable.
         * \param index The variable's index.
         * \return The variable's name.
         *
         * \sa IO::Block::getName()
         */
        std::string getName(IO::flags_t type,size_t index) const;
        /*!
         * Get the description of the specified variable.
         *
         * \param type The type of the variable.
         * \param index The variable's index.
         * \return The variable's description.
         *
         * \sa IO::Block::getDescription()
         */
        std::string getDescription(IO::flags_t type,size_t index) const;
        /*!
         * Get the value of the specified EVENT, PARAMETER or STATE variable.
         *
         * \param type The type of the specified variable.
         * \param index The variable's index.
         * \return The variable's value.
         *
         * \sa IO::Block::getValue()
         * \sa Workspace::setData()
         */
        double getValue(IO::flags_t type,size_t index) const;

        /*!
         * Set the value of a PARAMETER type variable.
         *
         * \param index The variable's index.
         * \param value The variable's new value.
         *
         * \sa Workspace::PARAMETER
         */
        void setValue(size_t index,double value);

    protected:

        /*!
         * Get the internal reference of the variable, for STATE types.
         *
         * \param type The variable's type.
         * \param index The variable's index.
         * \return The variable's storage location.
         *
         * \sa Workspace::STATE
         */
        double *getData(IO::flags_t type,size_t index);

        /*!
         * Set the internal reference of the variable, for STATE types.
         *
         * \param type The variable's type.
         * \param index The variable's index.
         * \param value The variable's storage location.
         *
         * \sa Workspace::STATE
         */
        void setData(IO::flags_t type,size_t index,double *value);

    private:

        typedef struct {
            std::string name;
            std::string description;
            double *data;
        } var_t;

        std::vector<var_t> parameter;
        std::vector<var_t> state;
        std::vector<var_t> event;

    }; // class Object

    /*!
     * Acts as a central meeting point between Instances. Provides
     *   interfaces for finding and manipulating Blocks.
     *
     * \sa Workspace::Instance
     */
    class Manager {

        friend class Instance;

    public:

        static void initialize(void);

        /*!
         * Loop through each Instance and execute a callback.
         * The callback takes two parameters, an Instance pointer and param,
         *   the second parameter to foreachWorkspace.
         *
         * \param callback The callback function.
         * \param param A parameter to the callback function.
         * \sa Workspace::Instance
         */
        static void foreachWorkspace(void (*callback)(Instance *,void *),void *param);

    private:

        /*****************************************************************
         * The constructor, destructor, and assignment operator are made *
         *   private to control instantiation of the class.              *
         *****************************************************************/

        Manager(void) : mutex(Mutex::RECURSIVE) {};
        ~Manager(void) {};
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *instance; };

        static Manager *instance;

        static void insertWorkspace(Instance *);
        static void removeWorkspace(Instance *);

        Mutex mutex;
        std::list<Instance *> instanceList;

    }; // class Manager

} // namespace Workspace

#endif // WORKSPACE_H
