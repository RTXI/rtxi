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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <event.h>
#include <fifo.h>
#include <io.h>
#include <plugin.h>
#include <qwidget.h>
#include <vector>

class QComboBox;
class QListBox;
class QPushButton;

namespace Connector {

    class Panel : public QWidget, Event::Handler {

        Q_OBJECT

    public:

        Panel(QWidget *);
        ~Panel(void);

        void receiveEvent(const Event::Object *);

    private slots:

        void buildInputChannelList(void);
        void buildOutputChannelList(void);
        void highlightConnectionBox(const QString &);
        void toggleConnection(bool);
        void updateConnectionButton(void);

    private:

        static void buildConnectionList(IO::Block *,size_t,IO::Block *,size_t,void *);

        struct link_t {
            IO::Block *src;
            size_t src_idx;
            IO::Block *dest;
            size_t dest_idx;
        };

        QComboBox *inputBlock;
        QComboBox *inputChannel;
        QComboBox *outputBlock;
        QComboBox *outputChannel;
        QListBox *connectionBox;
        QPushButton *connectionButton;
        std::vector<IO::Block *> blocks;
        std::vector<link_t> links;

    }; // class Panel

    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        friend class Panel;

    public:

        static Plugin *getInstance(void);

    public slots:

        void showConnectorPanel(void);

    private:

        void removeConnectorPanel(Panel *);

        Plugin(void);
        ~Plugin(void);
        Plugin(const Plugin &) {};
        Plugin &operator=(const Plugin &) { return *getInstance(); };

        static Plugin *instance;

        int menuID;
        Panel *panel;

    }; // class Plugin

}; // namespace Connector

#endif // CONNECTOR_H
