#include "CP_protocol.h"

#include <iostream>

using namespace std;

// Class ProtocolStep - base unit of a protocol
ClampProtocol::ProtocolStep::ProtocolStep()
 : ampMode(VOLTAGE), stepType(STEP), stepDuration(0), deltaStepDuration(0), holdingLevel1(0),
   deltaHoldingLevel1(0), holdingLevel2(0), deltaHoldingLevel2(0), pulseWidth(0), pulseRate(0) {
}

double ClampProtocol::ProtocolStep::retrieve(int row){
    switch(row){
    case 0: return ampMode;
        break;
    case 1: return stepType;
        break;
    case 2: return stepDuration;
        break;
    case 3: return deltaStepDuration;
        break;
    case 4: return holdingLevel1;
        break;
    case 5: return deltaHoldingLevel1;
        break;               
    case 6: return holdingLevel2;
        break;
    case 7: return deltaHoldingLevel2;
        break;
    case 8: return pulseWidth;
        break;
    case 9: return pulseRate;
    break;
    default:
        cout << "Error - ClampProtocol::ProtocolStep::retrieve() - default case";
        return 0;
        break;
    }
}

// Class ProtocolSegment - segment of a protocol, made up of ProtocolSteps
ClampProtocol::ProtocolSegment::ProtocolSegment()
  : numSweeps(1) {

}

// Class Protocol - Protocol used by clamp suite module
ClampProtocol::Protocol::Protocol() {
}

ClampProtocol::Step ClampProtocol::Protocol::getStep( int seg, int step ) {
    return protocolContainer.at( seg )->segmentContainer.at( step );
}

int ClampProtocol::Protocol::numSteps( int seg ) {
    return protocolContainer.at( seg )->segmentContainer.size();
}

int ClampProtocol::Protocol::addStep( int seg, int step ) {
    if( seg > numSegments() ) // If segment doesn't exist or not at end
        return 0;
    if( step > numSteps( seg ) ) // If step doesn't exist or not at end
        return 0;

    Segment segment = getSegment( seg );

    if( step == numSteps( seg ) ) {// Use push_back if at end of vector
        segment->segmentContainer.push_back( Step( new ProtocolStep ) );
        return 1;
    }
    else {
        SegmentContainerIt it = segment->segmentContainer.begin();
        segment->segmentContainer.insert( it + step, Step( new ProtocolStep ) );
        return 1;
    }
}

int ClampProtocol::Protocol::deleteStep( int seg, int step ) {
    if( seg > numSegments() ) // If segment doesn't exist or not at end
        return 0;
    if( step > numSteps( seg ) ) // If step doesn't exist or not at end
        return 0;

    Segment segment = getSegment( seg );

    SegmentContainerIt it = segment->segmentContainer.begin();
    segment->segmentContainer.erase( it + step );

    return 1;
}


int ClampProtocol::Protocol::addSegment( int seg ) {
    if( seg > numSegments() ) { // If segment doesn't exist or not at end
        return 0;
    }
    if( seg == numSegments() ) {
        protocolContainer.push_back( Segment( new ProtocolSegment ) );
        return 1;
    }
    else {
        ProtocolContainerIt it = protocolContainer.begin();
        protocolContainer.insert( it + seg, Segment( new ProtocolSegment ) );
        return 1;
    }

    return 0;
}

int ClampProtocol::Protocol::deleteSegment( int seg ) {
    if( seg > numSegments() ) { // If segment doesn't exist or not at end
        return 0;
    }

    ProtocolContainerIt it = protocolContainer.begin();
    protocolContainer.erase( it + seg );
    return 1;
}

int ClampProtocol::Protocol::numSweeps( int seg ) {
    return getSegment( seg )->numSweeps;
}

int ClampProtocol::Protocol::segmentLength( int seg, double period, bool withSweeps ) { // Period in ms
    int time = 0;
    
    if( withSweeps ) { // Length of all sweeps
        for( int i = 0; i < numSteps( seg ); i++ )
            for( int j = 0; j < numSweeps( seg ); j++ )
                time += getStep( seg, i )->stepDuration +  ( getStep( seg, i )->deltaStepDuration * j );
    }
    else { // Length of just first sweep
        for( int i = 0; i < numSteps( seg ); i++ )
            time += getStep( seg, i )->stepDuration;
    }
    
    return time / period;
}

void ClampProtocol::Protocol::setSweeps( int seg, int sweeps ) {
    getSegment( seg )->numSweeps = sweeps;
}

ClampProtocol::Segment ClampProtocol::Protocol::getSegment( int segNum ) { // Returns segment of a protocol
    return protocolContainer.at( segNum );
}

int ClampProtocol::Protocol::numSegments( void ) { // Returns number of segments in a protocol / size of protocol
    return protocolContainer.size();
}

QDomElement ClampProtocol::Protocol::stepToNode( QDomDocument &doc, int seg, int stepNum ) { // Converts protocol step to XML node
    QDomElement stepElement = doc.createElement("step"); // Step element
    Step step = getStep( seg, stepNum );

    // Set attributes of step to element
    stepElement.setAttribute( "stepNumber", QString::number(stepNum) );
    stepElement.setAttribute( "ampMode", QString::number(step->ampMode) );
    stepElement.setAttribute( "stepType", QString::number(step->stepType) );
    stepElement.setAttribute( "stepDuration", QString::number(step->stepDuration) );
    stepElement.setAttribute( "deltaStepDuration", QString::number(step->deltaStepDuration) );
    stepElement.setAttribute( "holdingLevel1", QString::number(step->holdingLevel1) );
    stepElement.setAttribute( "deltaHoldingLevel1", QString::number(step->deltaHoldingLevel1) );
    stepElement.setAttribute( "holdingLevel2", QString::number(step->holdingLevel2) );
    stepElement.setAttribute( "deltaHoldingLevel2", QString::number(step->deltaHoldingLevel2) );
    stepElement.setAttribute( "pulseWidth", QString::number(step->pulseWidth) );
    stepElement.setAttribute( "pulseRate", QString::number(step->pulseRate) );

    return stepElement;
}

QDomElement ClampProtocol::Protocol::segmentToNode( QDomDocument &doc, int seg ) { // Converts protocol segment to XML node
    QDomElement segmentElement = doc.createElement( "segment" ); // Segment element
    segmentElement.setAttribute( "numSweeps", numSweeps( seg ) );
    
    // Add each step as a child to segment element
    for( int i = 0; i < numSteps( seg ); i++ ) {
        if( getStep( seg, i ) != NULL ) // If step exists
            segmentElement.appendChild( stepToNode( doc,  seg, i ) );
    }
                
    return segmentElement; // Return segment element   
}

void ClampProtocol::Protocol::clear( void ) { // Clears protocol container
    protocolContainer.clear();
}

void ClampProtocol::Protocol::toDoc( void ) { // Convert protocol to QDomDocument   
    QDomDocument doc("ClampProtocolML");
    
    QDomElement root = doc.createElement( "Clamp-Suite-Protocol-v1.0");
    doc.appendChild(root);

    // Add segment elements to protocolDoc
    for( int i = 0; i < numSegments(); i++ ) {
        root.appendChild( segmentToNode( doc, i ) );
    }

    protocolDoc = doc; // Shallow copy
}

void ClampProtocol::Protocol::fromDoc( QDomDocument doc ) { // Load protocol from QDomDocument
    QDomElement root = doc.documentElement(); // Get root element from document
    
    // Retrieve information from document and set to protocolContainer
    QDomNode segmentNode = root.firstChild(); // Retrieve first segment
    clear(); // Clear vector containing protocol
    int segmentCount = 0;
    
    while( !segmentNode.isNull() ) { // Segment iteration
        QDomElement segmentElement = segmentNode.toElement();
        int stepCount = 0;
        addSegment( segmentCount ); // Add segment to protocol container
        getSegment( segmentCount )->numSweeps = segmentElement.attribute( "numSweeps" ).toInt();
        QDomNode stepNode = segmentNode.firstChild();
        
        while( !stepNode.isNull() ) {// Step iteration
            addStep( segmentCount, stepCount ); // Add step to segment container
            Step step = getStep( segmentCount, stepCount ); // Retrieve step pointer
            QDomElement stepElement = stepNode.toElement();
            // Retrieve attributes
            step->ampMode = ( ProtocolStep::ampMode_t )stepElement.attribute( "ampMode" ).toDouble();
            step->stepType = ( ProtocolStep::stepType_t )stepElement.attribute( "stepType" ).toDouble();
            step->stepDuration = stepElement.attribute( "stepDuration" ).toDouble();
            step->deltaStepDuration = stepElement.attribute( "deltaStepDuration" ).toDouble();
            step->holdingLevel1 = stepElement.attribute( "holdingLevel1" ).toDouble();
            step->deltaHoldingLevel1 = stepElement.attribute( "deltaHoldingLevel1" ).toDouble();            
            step->holdingLevel2 = stepElement.attribute( "holdingLevel2" ).toDouble();
            step->deltaHoldingLevel2 = stepElement.attribute( "deltaHoldingLevel2" ).toDouble();
            step->pulseWidth = stepElement.attribute( "pulseWidth" ).toDouble();
            step->pulseRate = stepElement.attribute( "pulseRate" ).toInt();
            
            stepNode = stepNode.nextSibling(); // Move to next step
            stepCount++;
        } // End step iteration
        
        segmentNode = segmentNode.nextSibling(); // Move to next segment
        segmentCount++;
    } // End segment iteration
}

std::vector< std::vector<double> > ClampProtocol::Protocol::run( double period ) {
    // Run the protocol and keep track of time (ms) and output (mv)
    // Return time and output vector, based off of clamp_protocol.cpp execute function

    std::vector<double> timeVector;
    std::vector<double> outputVector;

    enum protocolMode_t { SEGMENT, STEP, EXECUTE, END } protocolMode = SEGMENT;
    double time = 0;
    double output = 0;
    Step step;
    ProtocolStep::stepType_t stepType;
    int segmentIdx = 0; // Current segment
    int sweepIdx = 0; // Current sweep
    int stepIdx = 0; // Current step
    int sweeps = 0; // Number of sweeps for the current segment
    int steps = 0; // Number of steps in the current segment
    int stepTime = 0, stepEndTime = 0; // Time elapsed during the current step
    double stepOutput = 0;
    double rampIncrement = 0;
    double pulseWidth = 0;
    int pulseRate = 0;

    while( protocolMode != END ) {

        if( protocolMode == SEGMENT ) { // Segment initialization       
            sweeps = numSweeps( segmentIdx );
            steps = numSteps( segmentIdx );            
            protocolMode = STEP; // Move on to step initialization            
        } // end ( protocolMode == SEGMENT )

        if( protocolMode == STEP ) { // Step initialization
            step = getStep( segmentIdx, stepIdx ); // Retrieve step pointer
            stepType = step->stepType; // Retrieve step type
            stepTime = 0;
            
            // Initialize step variables
            stepEndTime = ( ( step->stepDuration + ( step->deltaStepDuration * (sweepIdx) ) ) / period ) - 1; // Unitless to prevent rounding errors
            stepOutput = step->holdingLevel1 + ( step->deltaHoldingLevel1 * (sweepIdx) );

            if( stepType == ProtocolStep::RAMP ) {
                double h2 = step->holdingLevel2 + ( step->deltaHoldingLevel2 * (sweepIdx) ); // End of ramp value
                rampIncrement = ( h2 - stepOutput ) / stepEndTime; // Slope of ramp
            }
            else if ( stepType == ProtocolStep::TRAIN ) {
                pulseWidth = step->pulseWidth / period; // Unitless to prevent rounding errors
                pulseRate = step->pulseRate / ( period * 1000 ); // Unitless to prevent rounding errors
            }
            
            protocolMode = EXECUTE; // Move on to tep execution    
        } // end ( protocolMode == STEP )
        
        if( protocolMode == EXECUTE ) {
            switch( stepType ) {
            case ProtocolStep::STEP:
                output = stepOutput;                
                break;
                
            case ProtocolStep::RAMP:
                output = ( stepOutput + (stepTime * rampIncrement) );
                break;
                
            case ProtocolStep::TRAIN:
                if( stepTime % pulseRate < pulseWidth )
                    output = stepOutput;
                else
                    output = 0;                
                break;
                
            default:
                break;
            } // end switch( stepType)

            stepTime++;
            
            if( stepTime > stepEndTime ) { // If step is finished
                stepIdx++;
                protocolMode = STEP;
                
                if( stepIdx == steps ) { // If done with all steps
                    sweepIdx++;
                    stepIdx = 0;
                    
                    if( sweepIdx == sweeps ) { // If done with all sweeps
                        segmentIdx++;
                        sweepIdx = 0;
                        
                        protocolMode = SEGMENT; // Move on to next segment

                        if( segmentIdx >= numSegments() ) {// If finished with all segments
                            protocolMode = END; // End protocol
                        }
                    }
                }
            } // end stepTime > stepEndTime                            
        } // end ( protocolMode == EXECUTE )        

        timeVector.push_back( time );
        outputVector.push_back( output );
        
        // Update States
        time += period;
    }

    std::vector< std::vector<double> > retval;
    retval.push_back( timeVector ); // ms
    retval.push_back( outputVector ); // mv

    return retval;
 }
