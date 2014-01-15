\newcommand{\whatsize}{10pt}
\newcommand{\tskip}{0ex} % Terminal Skip: distance (pt) to indent when inserting line that looks like it's from the terminal
\newcommand{\sskip}{1ex} % Source Skip: distance to indent (pt) when inserting a line of source code. Different from tskip because there's no "$ " prefix in source files. 
\newcommand{\dontindentlist}{[$\hspace{0pt}1.$]} %use this after \begin{enumerate} to prevent indentation of numbered lists. ([\dontindentlist])

\documentclass[\whatsize, final]{article}
\usepackage{enumerate, url, graphicx}
\graphicspath{{before/}{after/}{header/}{compile/}{source/}}

\begin{document}

\title{{\Large {\bfseries How to Add a Button to an RTXI Plugin}}\vspace{0ex}}
\author {\large {By the indefagitable RTXI elves}\vspace{0ex}}
\date {\large \today \vspace{0ex}}
\maketitle

\begin{flushleft}
\noindent This tutorial provides a guideline for adding buttons within an RTXI module. We will be using my\_plugin\_gui for starter code, and this tutorial will focus on adding a third button to the existing group of two. \\
\end{flushleft}

\begin{figure}[h!]
\centering
 \begin{minipage}{.45\linewidth}
 \centering
 \includegraphics[scale=.5]{before}
 \caption{Before}
 \label{fig:before}
 \end{minipage}
{\bfseries \huge ${\vcenter{\hbox{$\rightarrow$}}}$} \
 \begin{minipage}{.45\linewidth}
 \centering
 \includegraphics[scale=.5]{"RTXI - Real-time eXperimental Interface_016"}
 \caption{After}
 \label{fig:after}
 \end{minipage}
\end{figure}

\paragraph{Note}
\noindent Before we begin, please make sure that your RTXI installation is less than and \emph{not} equal to version 2.0. Versions 2.0 and above rely on Qt4. This tutorial utilizes code that is compatible with Qt3 libraries and should not be expected to advocate error-free code if used as a starting point for plugins written for Qt4. 

\section*{Preparation:}
\begin{enumerate}[$\hspace{0pt}1.$]
\item Create a new directory and call it add\_button.\\
\hspace*{\tskip} {\sffamily \$ \ mkdir \ add\_button}

\item Copy the files from my\_plugin\_gui and place them in add\_button. \\
\hspace*{\tskip} {\sffamily \$ \ cp \ \url{~}/modules/my\_plugin\_gui/* \ \url{~}/PATH\_TO/add\_button/}

\item Move to the directory and change the filenames my\_plugin\_gui.cpp and my\_plugin\_gui.h to add\_button.cpp and add\_button.h, respectively. \\
\hspace*{\tskip} {\sffamily \$ \ mv \ my\_plugin\_gui.cpp \ add\_button\_.cpp} \\
\hspace*{\tskip} {\sffamily \$ \ mv \ my\_plugin\_gui.h \ add\_button\_.h}

\item Alter the makefile, source file, and header file to reflect the file name changes. \\[\whatsize]
Run the line below within the add\_button directory. The sed command searches for all user-specified files for a string that, if found, is replaced by another user-specified string. The command below will search the directory for all instances of ``my\_plugin\_gui" and replace them with ``add\_button".\\
\hspace*{\tskip} {\sffamily \$ \ sed \ -i \ ``s/my\_plugin\_gui/add\_button/g" \ Makefile \ add\_button.cpp \ add\_button.h }

\begin{figure}[h!]
\centering
 \includegraphics[scale=.4]{"Makefile before sed"}
 \caption{The contents of Makefile prior to modification}
%\end{figure}
%\begin{figure}[h!]
% \centering
 \includegraphics[scale=.4]{"Makefile after sed"}
 \caption{The contents after the sed command is run. Note all instances of my\_plugin\_gui have changed to add\_button.}
\end{figure}

\end{enumerate}

\section*{Header File:}
\begin{enumerate}[$\hspace{0pt}1.$]
\item Open add\_button.h. The screenshots in the tutoral all use vim, but feel free to use whatever text editor or IDE you prefer. 

\item Within the ``private slots" section of the file, add a function called cBttn\_event. \\
\hspace*{\sskip} {\sffamily void \ cBttn\_event(void);} \\[\whatsize]
This function will be associated with the button we will create in the GUI. In other words, the function, whose contents will be defined in the source file, will be called each time the button we will associate it with is pressed.

\begin{figure}[h!]
 \centering
 \includegraphics[scale=.5]{"my_plugin_gui_cropped"}
 \caption{The contents of my\_plugin\_gui.h prior to modification}
%\end{figure}
%\begin{figure}[h!]
% \centering
 \includegraphics[scale=.5]{"add_button_crop"}
 \caption{A declaration for the function cBttn\_event() is added to the header file.}
\end{figure}

\item Save your changes and close the file.
\end{enumerate}

\section*{Source File:}
\begin{enumerate}[$\hspace{0pt}1.$]
\item Open add\_button.cpp.

\item Go to around line 107 and begin a new line. 
Enter: \\
\hspace*{\sskip} {\sffamily QPushButton \ cBttn \ = \ new \ QPushButton(``Button C", bttnGroup);} \\[\whatsize]
QPushButton is a Qt class that creates a button that undergoes an animation when clicked that gives the appearance of it being pushed down and back, like a real-life button. The initialization takes two arguments: 
 \begin{enumerate}[$\hspace{5pt}i.$]
 \item A string that will be displayed on the button on the GUI. The button's label, essentially.
 \item The parent object for the button.
 \end{enumerate}
In our case, cBttn is a new QPushButton that is a child of the bttnGroup widget. If you read a few lines above, you can see bttnGroup's initialization. bttnGroup is a QHButtonGroup object, meaning that it is a Qt-defined widget that puts buttons in a horizontal row in the order in which they are added. By default, QHButtonGroup handles button spacing, widget size, etc., but behavior can also be specified by the user. This tutorial will not cover that. 

\item Skip a few lines down to around line 113. Add the following:\\
\hspace*{\sskip} {\sffamily QObject::connect(cBttn, SIGNAL(clicked()), this, SLOT(cBttn\_event()));}\\

This line connects the clicking of cBttn to the function cBttn\_event. The connect function's namespace is not included in the file or the header, so the ``QObject::" prefix is necessary. The arguments are, in sequential order:
 \begin{enumerate}[$\hspace{5pt}i.$]
 \item The name of the object to be connected (cBttn)
 \item The signal type (SIGNAL(type of signal)) sent by the object
 \item The Qt object that receives the signal, which in this case is the object itself, so `this' is used. 
 \item The response of the receiving object (call cBttn\_event). SIGNAL()s are received by SLOT()s, so the expression needed is SLOT(cBttn\_event()). 
 \end{enumerate}

\begin{figure}[h!]
 \centering
 \includegraphics[scale=.5]{"add_button_crop_004"}
 \caption{This is where the new code will be added}
 \includegraphics[scale=.5]{"add_button_crop_005"}
 \caption{Add the connect() function and QPushButton it connects}
\end{figure}

\item Skip to the very bottom of the file and define the cBttn\_event function. You can use whatever you like, but our main goals is to simply create a button. Therefore, the function definition can be left empty. This will result in button clicks not doing anything. \\
\hspace*{\sskip} {\sffamily void cBttn\_event(void) \{\};} \\[\whatsize]
cBttn\_event will still be called whenever a click occurs. We simply have made it so each call to cBttn\_event will not do anything. 

\begin{figure}[h!]
 \centering
 \includegraphics[scale=.5]{"add_button_crop_007"}
 \caption{Add the definition of the cBttn\_event function. Leave it void.}
\end{figure}

\item Save changes and close the file. 
\end{enumerate}

\section*{Compilation:}
\begin{enumerate}[$\hspace{0pt}1.$]
\item Compile the plugin from within the add\_button directory. Run: \\
This will compile and install the software. If a compilation error occurs, check the source files for types. This is a simple plugin, so complex errors are not to be expected. 

\begin{figure}[h!]
 \centering
 \includegraphics[scale=.5]{"module compiled"}
 \caption{This is what the terminal outputs upon successful compilation.}
\end{figure}

\end{enumerate}

\section*{Running in RTXI:}
\begin{enumerate}[$\hspace{0pt}1.$]
\item Open RTXI. 
\item Go the the top menubar and select ``Modules" from the menu. It is second from the left. 
\item Go th ``Load user module."
\item Select your module from the list of available ones. It will be named ``add\_button.so".
\item View the new button.

\begin{figure}[h!]
 \centering
 \includegraphics[scale=.5]{"RTXI - Real-time eXperimental Interface_016"}
 \caption{This is the result of the previous code additions.}
\end{figure}

\end{enumerate}

\paragraph{Concluding Remarks} 
This completes the button-adding tutorial. Granted, there are many options to explore that have not been explained or addressed. Development for RTXI is ongoing, and user feedback on this tutorial and/or suggestions for future ones is greatly appreciated and will be addressed with due speed.
\end{document}