#ifndef CP_PROTOCOL_EDITOR_H
#define CP_PROTOCOL_EDITOR_H


#include "CP_protocol.h"
#include "ui/CP_protocol_editorUI.h"

#include <qtable.h>
#include <qstringlist.h>

namespace ClampProtocol {
    // No function to change alignment in Qt3.3, so subclassing must be done
    class CenterAlignTableItem : public QTableItem {
    public:
        CenterAlignTableItem( QTable *, EditType );
        int alignment() const;
    };

    class ProtocolEditor : public ProtocolEditorUI { // QWidget dialog, inherits Qt Designer designed GUI
        Q_OBJECT
        public: 
        ProtocolEditor( QWidget * );
        ~ProtocolEditor( void ) { };
        Protocol protocol; // Clamp protocol
                         
    private:
        int currentSegmentNumber;
        QStringList ampModeList, stepTypeList;
        void createStep( int );
        int loadFileToProtocol( QString );
        bool protocolEmpty( void );

    signals:
        void protocolTableScroll( void );
                        
    public slots:
        QString loadProtocol( void );
        void loadProtocol( QString );
        void clearProtocol( void );
        void exportProtocol( void );
        void previewProtocol( void );
        
    private slots:
        void addSegment( void );
        void deleteSegment( void );
        void addStep( void );
        void insertStep( void );
        void deleteStep( void );
        void updateSegment( QListViewItem* );
        void updateSegmentSweeps( int );
        void updateTableLabel( void );
        void updateTable( void );
        void updateStepAttribute( int, int );
        void updateStepType( int, ProtocolStep::stepType_t );
        void saveProtocol( void );
    };    
}; // namespace ClampProtocol

#endif // CP_protocol_editor.h
