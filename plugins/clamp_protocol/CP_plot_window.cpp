#include "CP_plot_window.h"
#include "clamp_protocol.h"

#include <qlayout.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qscrollview.h>
#include <qsizepolicy.h>
#include <qtooltip.h>
#include <qwt-qt3/qwt_text.h>
#include <qwt-qt3/qwt_legend.h>

ClampProtocol::PlotWindow::PlotWindow( QWidget *parent, Panel *p )
    : PlotWindowUI( parent, "Plot Window", Qt::WDestructiveClose ), panel( p ), overlaySweeps( false ),
      plotAfter( false ), colorScheme( 0 ),  runCounter( 0 ), sweepsShown( 0 ) {

    QVBoxLayout *plotWindowUILayout = new QVBoxLayout( this );
    plot = new BasicPlot( this );
    
    // Add scrollview for top part of widget to allow for smaller widths
    QScrollView *sv = new QScrollView( this );
    sv->addChild( frame ); // UI contains a frame not bound to a layout, this is added to scroll view
    sv->setResizePolicy( QScrollView::AutoOneFit ); // Makes sure frame is the size of scrollview
    sv->setVScrollBarMode( QScrollView::AlwaysOff );
    sv->setFixedHeight( 85 );
    plot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    plotWindowUILayout->addWidget( sv );
    plotWindowUILayout->addWidget( plot );

    resize( 625, 400 ); // Default size
    
    // Plot settings
    font = timeScaleLabel->font(); // Use label font as template
    font.setPixelSize(12);

    QwtText xAxisTitle, yAxisTitle;
    xAxisTitle.setText( "Time (ms)" );
    xAxisTitle.setFont( font );
    yAxisTitle.setText( "Current (nA)" );
    yAxisTitle.setFont( font );
    plot->setAxisTitle( QwtPlot::xBottom, xAxisTitle );
    plot->setAxisTitle( QwtPlot::yLeft, yAxisTitle );
    setAxes(); // Set axes to defaults (1,1000)(-20,20)

    QwtLegend *legend = new QwtLegend();
    plot->insertLegend( legend, QwtPlot::RightLegend );

    // Signal/Slot Connections
    QObject::connect( setAxesButton, SIGNAL(clicked(void)), this, SLOT(setAxes(void)) );
    QObject::connect( timeX1Edit, SIGNAL(valueChanged(int)), this, SLOT(setAxes(void)) );
    QObject::connect( timeX2Edit, SIGNAL(valueChanged(int)), this, SLOT(setAxes(void)) );
    QObject::connect( currentY1Edit, SIGNAL(valueChanged(int)), this, SLOT(setAxes(void)) );
    QObject::connect( currentY2Edit, SIGNAL(valueChanged(int)), this, SLOT(setAxes(void)) );
    QObject::connect( clearButton, SIGNAL(clicked(void)), this, SLOT(clearPlot(void)) );
    QObject::connect( overlaySweepsCheckBox, SIGNAL(clicked(void)), this, SLOT(toggleOverlay(void)) );
    QObject::connect( plotAfterCheckBox, SIGNAL(clicked(void)), this, SLOT(togglePlotAfter(void)) );
    QObject::connect( colorByComboBox, SIGNAL(activated(int)), this, SLOT(changeColorScheme(int)) );

    font.setBold( false );

    // Add tooltip to color scheme combo box
    QString tooltip =
        QString( "There are 10 colors which rotate in the same order\n" ) +
        QString( "Run: Change color after every protocol run\n" ) +
        QString( "Trial: For use when running multiple trials - A color will correspond to a specific trial number\n" ) +
        QString( "Sweep: A color will correspond to a specific sweep" );
    QToolTip::add( colorByComboBox, tooltip );
}

ClampProtocol::PlotWindow::~PlotWindow( void ) {
    panel->removePlotWindow( this );
}

void ClampProtocol::PlotWindow::addCurve( double *output, curve_token_t token ) { // Attach curve to plot
   double time[ token.points ];

   if( overlaySweeps ) 
       for( size_t i = 0; i < token.points; i++ )
           time[ i ] = token.period * ( token.stepStartSweep + i );   
   else 
       for( size_t i = 0; i < token.points; i++ ) 
           time[ i ] = token.period * ( token.stepStart + i );                          
   
   if( token.stepStart == -1 ) // stepStart is offset by -1 in order to connect curves, but since i is unsigned, must be careful of going negative  
       time[ 0 ] = 0;
   
   int idx;
   QString curveTitle;

   bool legendShow = token.lastStep; // Whether legend entry will be added
   
   switch( colorScheme ) {
   case 0: // Color by Run
       idx = runCounter % 10;
       curveTitle = "Run " + QString::number( runCounter + 1 );

       if( token.lastStep ) // Increase run counter if curve is last step in a run
           runCounter++;              
       break;
       
   case 1: // Color by Trial
       idx = token.trial;
       curveTitle = "Trial " + QString::number( idx + 1 );
       break;
       
   case 2: // Color by sweep
       idx = token.sweep;

       if( idx >= sweepsShown ) {
           legendShow = true;
           sweepsShown++;
       }
       else
           legendShow = false;
       
       curveTitle = "Sweep " + QString::number( idx + 1 );
       break;
       
   default:
       break;
   }   
   
   curveContainer.push_back( QwtPlotCurvePtr( new QwtPlotCurve(curveTitle) ) );
   QwtPlotCurvePtr curve = curveContainer.back();
   curve->setData( time, output, token.points ); // Makes a hard copy of both time and output
   colorCurve( curve, idx );
   curve->setItemAttribute( QwtPlotItem::Legend, legendShow ); // Set whether curve will appear on legend
   curve->attach( plot );
   if( legendShow ) {
       plot->legend()->legendItems().back()->setFont( font ); // Adjust font
   }

   if( plotAfter && !token.lastStep ) // Return before replot if plotAfter is on and its not last step of protocol
       return ;
   
   plot->replot(); // Attaching curve does not refresh plot, must replot
}

void ClampProtocol::PlotWindow::colorCurve( QwtPlotCurvePtr curve, int idx ) {
    QColor color;
        
    switch( idx ) {
    case 0: color = QColor( Qt::black ); break;
    case 1: color = QColor( Qt::red ); break;
    case 2: color = QColor( Qt::blue ); break;
    case 3: color = QColor( Qt::green ); break;
    case 4: color = QColor( Qt::cyan ); break;
    case 5: color = QColor( Qt::magenta ); break;
    case 6: color = QColor( Qt::yellow ); break;
    case 7: color = QColor( Qt::lightGray ); break;
    case 8: color = QColor( Qt::darkRed ); break;
    case 9: color = QColor( Qt::darkGreen ); break;
    default: color = QColor( Qt::black ); break;
    }

    QPen pen( color, 2 ); // Set color and width
    curve->setPen( pen );
}

void ClampProtocol::PlotWindow::setAxes( void ) {    
    double timeFactor, currentFactor;
    
    switch( timeScaleEdit->currentItem() ) { // Determine time scaling factor, convert to ms
    case 0: timeFactor = 10; // (s)
        break;
    case 1: timeFactor = 1; // (ms) default
        break;
    case 2: timeFactor = 0.1; // (us)
        break;
    default: timeFactor = 1; // should never be called
        break;
    }
    
    switch( currentScaleEdit->currentItem() ) { // Determine current scaling factor, convert to nA
    case 0: currentFactor = 10; // (uA)
        break;
    case 1: currentFactor = 1; // (nA) default
        break;
    case 2: currentFactor = 0.1; // (pA)
        break;
    default: currentFactor = 1; // shoudl never be called
        break;
    }
    
// Retrieve desired scale
    double x1, x2, y1, y2;
    
    x1 = timeX1Edit->value() * timeFactor;
    x2 = timeX2Edit->value() * timeFactor;
    y1 = currentY1Edit->value() * currentFactor;
    y2 = currentY2Edit->value() * currentFactor;
    
    plot->setAxes( x1, x2, y1, y2 );
}

void ClampProtocol::PlotWindow::clearPlot( void ) {
    curveContainer.clear();
    plot->replot();
}

void ClampProtocol::PlotWindow::toggleOverlay( void ) {    
    if( overlaySweepsCheckBox->isChecked() ) { // Checked
    // Check if curves are plotted, if true check if user wants plot cleared in
    // order to overlay sweeps during next run
    overlaySweeps = true;
    }
    else { // Unchecked
        overlaySweeps = false;
    }
}

void ClampProtocol::PlotWindow::togglePlotAfter( void ) {
    if( plotAfterCheckBox->isChecked() ) // Checked
        plotAfter = true;    
    else  // Unchecked
        plotAfter = false;

    plot->replot(); // Replot since curve container is cleared    
}

void ClampProtocol::PlotWindow::changeColorScheme( int choice ) {
    if( choice == colorScheme ) // If choice is the same
        return ;    
    
    // Check if curves are plotted, if true check if user wants plot cleared in
    // order to change color scheme
    if ( !curveContainer.empty() &&
         QMessageBox::warning(
                              this,
                              "Warning",
                              "Switching the color scheme will clear the plot.\nDo you wish to continue?",
                              QMessageBox::Yes | QMessageBox::Default, QMessageBox::No
                              | QMessageBox::Escape) != QMessageBox::Yes) {
        
        colorByComboBox->setCurrentItem( colorScheme ); // Revert to old choice if answer is no
        return ;
    }
    
    colorScheme = choice;
    curveContainer.clear();
    plot->replot(); // Replot since curve container is cleared
}

void  ClampProtocol::PlotWindow::doDeferred( const Settings::Object::State &s ) { }

void  ClampProtocol::PlotWindow::doLoad( const Settings::Object::State &s ) {
    if ( s.loadInteger("Maximized") )
        showMaximized();
    else if ( s.loadInteger("Minimized") )
        showMinimized();

    // Window Position
    if ( s.loadInteger( "W" ) != NULL ) {
        resize( s.loadInteger("W"), s.loadInteger("H") );
        parentWidget()->move( s.loadInteger("X"), s.loadInteger("Y") );
    }

    // Load Parameters
    timeX1Edit->setValue( s.loadInteger("X1") );
    timeX2Edit->setValue( s.loadInteger("X2") );
    timeScaleEdit->setCurrentItem( s.loadInteger("Time Scale") );
    currentY1Edit->setValue( s.loadInteger("Y1") );
    currentY2Edit->setValue( s.loadInteger("Y2") );
    currentScaleEdit->setCurrentItem( s.loadInteger("Current Scale") );
    overlaySweepsCheckBox->setChecked( s.loadInteger("Overlay Sweeps") );
    plotAfterCheckBox->setChecked( s.loadInteger("Plot After") );
    colorByComboBox->setCurrentItem( s.loadInteger("Color Scheme") );
    changeColorScheme( s.loadInteger("Color Scheme") );
    setAxes();
    toggleOverlay();
    togglePlotAfter();
    
}

void  ClampProtocol::PlotWindow::doSave( Settings::Object::State &s ) const {
    if ( isMaximized() )
        s.saveInteger( "Maximized", 1 );
    else if ( isMinimized() )
        s.saveInteger( "Minimized", 1 );

    // Window Position
    QPoint pos = parentWidget()->pos();
    s.saveInteger( "X", pos.x() );
    s.saveInteger( "Y", pos.y() );
    s.saveInteger( "W", width() );
    s.saveInteger( "H", height() );

    // Save parameters
    s.saveInteger( "X1", timeX1Edit->value() );
    s.saveInteger( "X2", timeX2Edit->value() );
    s.saveInteger( "Time Scale", timeScaleEdit->currentItem() );
    s.saveInteger( "Y1", currentY1Edit->value() );
    s.saveInteger( "Y2", currentY2Edit->value() );
    s.saveInteger( "Current Scale", currentScaleEdit->currentItem() );
    s.saveInteger( "Overlay Sweeps", overlaySweepsCheckBox->isChecked() );
    s.saveInteger( "Plot After", plotAfterCheckBox->isChecked() );
    s.saveInteger( "Color Scheme", colorByComboBox->currentItem() );
}
