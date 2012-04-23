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

#ifndef DEFAULT_GUI_MODEL_H
#define DEFAULT_GUI_MODEL_H

#include <event.h>
#include <map>
#include <mutex.h>
#include <plugin.h>
#include <qlineedit.h>
#include <rt.h>
#include <workspace.h>

class QLabel;
class QPushButton;

class DefaultGUILineEdit : public QLineEdit {

    Q_OBJECT

public:

    DefaultGUILineEdit(QWidget *);
    ~DefaultGUILineEdit(void);

    void blacken(void);

public slots:

    void redden(void);

}; // class DefaultGUILineEdit

/*!
 * A class that provides a simplified C++ interface for model creation.
 */
class DefaultGUIModel : public QWidget, public RT::Thread, public Plugin::Object, public Workspace::Instance, public Event::Handler
{

    Q_OBJECT

public:

    /*!
     * Flag that marks a variable as an input.
     *
     * \sa Workspace::INPUT
     */
    static const IO::flags_t INPUT     = Workspace::INPUT;
    /*!
     * Flag that marks a variable as an output.
     *
     * \sa Workspace::OUTPUT
     */
    static const IO::flags_t OUTPUT    = Workspace::OUTPUT;
    /*!
     * Flag that marks a variable as a parameter.
     *
     * \sa Workspace::PARAMETER
     */
    static const IO::flags_t PARAMETER = Workspace::PARAMETER;
    /*!
     * Flag that marks a variable as a state.
     *
     * \sa Workspace::STATE
     */
    static const IO::flags_t STATE     = Workspace::STATE;
    /*!
     * Flag that marks a variable as an event.
     *
     * \sa Workspace::STATE
     */
    static const IO::flags_t EVENT     = Workspace::EVENT;
    /*!
     * Flag that marks a variable as a comment.
     *
     * \sa Workspace::COMMENT
     */
    static const IO::flags_t COMMENT   = Workspace::COMMENT;
    /*!
     * Flag that marks a parameter as being of double type.
     */
    static const IO::flags_t DOUBLE    = Workspace::COMMENT<<1;
    /*!
     * Flag that marks a parameter as being of integer type.
     */
    static const IO::flags_t INTEGER   = Workspace::COMMENT<<2;
    /*!
     * Flag that marks a parameter as being of unsigned integer type.
     */
    static const IO::flags_t UINTEGER  = Workspace::COMMENT<<3;

    /*!
     * Structure used to pass variable information to the constructor
     *
     * \sa DefaultGUIModel::DefaultGUIModel()
     * \sa Workspace::variable_t
     */
    typedef Workspace::variable_t variable_t;

    /*!
     * Flag passed to DefaultGUIModel::update to signal the kind of update.
     *
     * \sa DefaultGUIModel::update()
     */
    enum update_flags_t {
        INIT,     /*!< The parameters need to be initialized.         */
        MODIFY,   /*!< The parameters have been modified by the user. */
        PERIOD,   /*!< The system period has changed.                 */
        PAUSE,    /*!< The Pause button has been activated            */
        UNPAUSE,  /*!< When the pause button has been deactivated     */
        EXIT,     /*!< When the module has been told to exit        */
    };

    /*!
     * The constructor needs to be provided with a specification of the
     *   variables that will be embedded in the enclosed workspace.
     *
     * \sa Workspace::Instance::Instance()
     * \sa DefaultGUIModel::variable_t
     */
    DefaultGUIModel(std::string name,variable_t *variables,size_t size);
    virtual ~DefaultGUIModel(void);

    /*!
     * Callback function that is called when the system state changes.
     *
     * \param flag The kind of update to signal.
     *
     * \sa DefaultGUIModel::update_flags_t
     */
    virtual void update(update_flags_t flag);

    /*!
     * Function that builds the Qt GUI.
     *
     * \param var The structure defining the module's parameters, states, inputs, and outputs.
     * \param size The size of the structure vars.
     *
     * \sa DefaultGUIModel::update_flags_t
     */
	void
	createGUI(DefaultGUIModel::variable_t *var, int size);

    QPushButton *pauseButton;
    struct param_t {
        QLabel *label;
        DefaultGUILineEdit *edit;
        IO::flags_t type;
        size_t index;
        QString *str_value;
    };
    std::map<QString,param_t> parameter;


public slots:

    /*!
     * Function that allows the object to safely delete and unload itself.
     */
    virtual void exit(void);
    /*!
     * Function that updates the GUI with new parameter values.
     *
     * \sa DefaultGUIModel::update_flags_t
     */
    virtual void refresh(void);
    /*!
     * Function that calls DefaultGUIModel::update with the MODIFY flag
     *
     * \sa DefaultGUIModel::update_flags_t
     */
    virtual void modify(void);
    /*!
     * Function that pauses/unpauses the model.
     */
    virtual void pause(bool);

protected:

    /*!
     * Get the value of the parameter in the GUI, and update the value
     *   within the Workspace.
     *
     * \param name The parameter's name.
     * \return The value of the parameter.
     */
    QString getParameter(const QString &name);
    /*!
     * Set the value of this parameter within the Workspace and GUI.
     *
     * \param name The name of the parameter.
     * \param ref A reference to the parameter.
     *
     * \sa Workspace::setData()
     */
    void setParameter(const QString &name,double value);
    /*!
     * Set the value of this parameter within the Workspace and GUI.
     *
     * \param name The parameter's name.
     * \param value The parameter's new value.
     */
    void setParameter(const QString &name,const QString value);
    /*!
     *
     */
    QString getComment(const QString &name);
    /*!
     *
     */
    void setComment(const QString &name,const QString comment);
    
    /*!
     * Set the reference to this state within the Workspace
     *   via Workspace::setData().
     *
     * \param name The state's name.
     * \param ref A reference to the state.
     *
     * \sa Workspace::setData()
     */
    void setState(const QString &name,double &ref);
    /*!
     * Set the reference to this event within the Workspace
     *   via Workspace::setData().
     *
     * \param name The event name.
     * \param ref A reference to the event.
     *
     * \sa Workspace::setData()
     */
    void setEvent(const QString &name,double &ref);

private:

    void doDeferred(const Settings::Object::State &);
    void doLoad(const Settings::Object::State &);
    void doSave(Settings::Object::State &) const;

    void receiveEvent(const Event::Object *);

    bool periodEventPaused;
    mutable QString junk;

    std::string myname;

}; // class DefaultGUIModel

#endif // DEFUALT_GUI_MODEL_H
