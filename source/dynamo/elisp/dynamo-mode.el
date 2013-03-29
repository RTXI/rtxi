;;
;;
;; dynamo-mode.el -- an Emacs mode for editing Dynamo model
;; description files. Derives from c-mode.
;;
;; Copyright (C) 2005 Ivan Raikov <raikov@cc.gatech.edu> and the
;; Georgia Institute of Technology. 
;;
;; This file is part of Dynamo.
;;
;; Dynamo is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; Dynamo is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with MRCI; see the file LICENSE.  If not, write to the Free
;; Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
;;
;;
;; Description
;; ===========
;;
;; This is a major mode for composing Dynamo system description files
;; and compiling them to object code using the Dynamo translator and the
;; Dynamo system make file.
;;
;; This mode offers several features to aid composing of Dynamo system
;; description files, including syntax colorization using either
;; font-lock, a syntax table appropriate to Dynamo, and key bindings.
;;
;; Once the system description file is composed, there is a function
;; for compiling the script. The interaction with the Dynamo translator
;; and the C compiler is within a comint buffer.
;;
;; The following key strokes are recognized:
;;
;;    C-c m f   --  inserts a Dynamo function declaration
;;    C-c m t   --  inserts a Dynamo table function declaration
;;    C-c m p   --  inserts a Dynamo parameter declaration
;;    C-c m s   --  inserts a Dynamo state declaration
;;    C-c m c   --  compiles the Dynamo system description in the
;;                  current buffer
;;
;; Entering Dynamo mode runs the hook `c-mode-common-hook', then
;; `dynamo-mode-hook'. The customizable variable `dynamo-compile-command'
;; contains the command to compile Dynamo system description files.
;; Each buffer in Dynamo mode has its own local copy of
;; `dynamo-compile-command'.
;; 
;;
;; Installation
;; ============
;;
;; By default, the Dynamo configure script places this file in
;; `/usr/local/share/emacs/site-lisp'. This can be changed by the user
;; by using the `--prefix' option. See the configure script help
;; information for further details. You'll need to make sure that the
;; directory where this file is placed is listed in the `load-path'
;; variable. See the `load-path' documentation for details.
;;
;;
;; To autoload dynamo-mode on any file with .dynamo extension, put this in 
;; your .emacs file
;;
;;          (require 'dynamo-mode)
;;
;; This module has been tested with GNU Emacs 21.2 and 20.7.
;;


(require 'cc-mode)

(provide 'dynamo-mode)

(defgroup dynamo nil
  "Editing mode for Dynamo system description files"
  :group 'languages)


(defcustom dynamo-mode-hook nil
  "Hook run by command `dynamo-mode'.
`c-mode-common-hook' is run first."
  :group 'dynamo
  :type 'hook)


(defcustom dynamo-compile-command "make -f /usr/local/share/dynamo/Makefile.dynamocompile" 
  "*The command to compile a Dynamo system description file 
The file name of current buffer file name (with suffix changed to `.o')
will be appended to this string, separated by a space."  
  :type 'string
  :group 'dynamo)

(defvar dynamo-cond-start nil)

(defconst dynamo-font-lock-keywords
  (eval-when-compile
    (list
     ;;
     ;; State function declarations.
     (list (concat "^\\s-*"
		   "\\<\\(\\(\\(TABLE\\|STATE\\|VECTOR\\)\\s-+\\)FUNCTION\\)\\>"
		   "\\s-+\\(\\(\\sw+\\|\\s-*,\\s-*\\)+\\)")
       '(1 font-lock-keyword-face nil nil) 
       '(3 font-lock-function-name-face nil nil))
     ;;
     ;; Event names.
     '("^\\s-*\\<\\(EVENT\\\\)\\>\\s-+\\(\\sw+\\)"
       (1 font-lock-keyword-face nil nil) 
       (2 font-lock-variable-name-face nil t))
     ;;
     ;; Time names.
     '("^\\s-*\\<\\(TIME\\|AT\\s-+TIME\\)\\>\\s-+\\(\\sw+\\)"
       (1 font-lock-keyword-face nil nil) 
       (2 font-lock-variable-name-face nil t))
     ;;
     ;; System names.
     '("^\\s-*\\(SYSTEM\\)\\>\\s-+\\(\\sw+\\)"
       (1 font-lock-keyword-face nil nil) 
       (2 font-lock-type-face nil nil))
     ;;
     ;; Parameter (constant) names.
     '("^\\s-*\\<PARAMETER\\>" (0 font-lock-keyword-face nil nil) 
       ("\\<\\sw+\\>" nil nil (0 font-lock-constant-face nil nil)))
     ;;
     ;; State names.
     '("^\\s-*\\<\\(\\(INTEGER\\|VECTOR\\|DISCRETE\\)\\s-*\\)*?STATE\\|EXTERNAL\\s-+\\(INPUT\\|OUTPUT\\)\\>" 
       (0 font-lock-keyword-face nil nil) 
       ("\\<METHOD\\>" (setq state-decl-start (point)) nil (0 font-lock-keyword-face nil nil))
       ("\\(\\<\\sw+\\>\\)+" (goto-char state-decl-start) nil (0 font-lock-variable-name-face nil nil)))

     ;; Conditional clauses in discrete equations
     '("\\(\\<CONDITION\\>\\)\\s-+\\[" (1 font-lock-keyword-face nil nil))
     (list (concat (regexp-quote "=>") "\\s-*\\(\\w+\\)") '(1 font-lock-constant-face nil t))
     
     ;; Case-based equations
     '("^\\s-*\\(\\<CASE\\>\\)\\s-+\\(\\sw+\\)\\s-+\\(!=\\|==\\)\\s-+\\(\\sw+\\)"  
       (1 font-lock-keyword-face nil t)
       (2 font-lock-variable-name-face nil t)
       (4 font-lock-constant-face nil t))
     
     ;; Differential equations
     '("^\\s-*\\(\\<d\\>\\)\\s-*(\\s-*\\(\\sw+\\)\\s-*)\\s-*="  
       (1 font-lock-keyword-face nil t)
       (2 font-lock-variable-name-face nil t))
     
     ;; Difference equations
     '("^\\s-*\\(\\<q\\>\\)\\s-*(\\s-*\\(\\sw+\\)\\s-*)\\s-*="  
       (1 font-lock-keyword-face nil t)
       (2 font-lock-variable-name-face nil t))

     ;; Discrete equations 1
     '("^\\s-*\\(\\<s\\>\\)\\s-*(\\s-*\\(\\sw+\\)\\s-*)\\s-*=\\s-*\\(\\sw+\\)"  
       (1 font-lock-keyword-face nil t)
       (2 font-lock-variable-name-face nil t)
       (3 font-lock-constant-face nil t))

     ;; Discrete equations 2
     '("^\\s-*\\(\\<s\\>\\)\\s-*(\\s-*\\(\\sw+\\)\\s-*)\\s-*="  
       (1 font-lock-keyword-face nil t)
       (2 font-lock-variable-name-face nil t))

     ;; Keywords.
     (regexp-opt '(
		   "DEFAULT" "METHOD" "LOW" "HIGH" "STEP" "ARGUMENT"
		   ) 'words)
     ;;
     ;; Builtins.
     (list (regexp-opt
	    '(
	      "cosh" "tanh" "sin" "cos" "tan" "ceil" "floor" "atan" 
	      "asin" "acos" "abs" "fabs" "log" "log10" "exp" "sqrt"
	      "sqr" "cube" "heaviside" "atan2" "max" "min" 
	      ) 'words) 
	   1
	   'font-lock-builtin-face) 
     ;;
     ;; Operators.  
     (cons (regexp-opt '("+" "*" "/" "-" "#" "^" "&&" "||" "<=" "<"
">=" ">" "==" "!=" "=" ";" "?" ":")) 'font-lock-keyword-face)))
  "Default expressions to highlight in Dynamo mode.")

(defun dynamo-insert-declaration (type name)
  "Insert a Dynamo declaration" 
  (save-excursion 
    (if (re-search-backward "AT\\s-+TIME" nil t) 
	(error "%s" "Definitions are not allowed within time blocks.")))

  (beginning-of-line)

  (save-excursion
    (save-restriction
      (let ((anchor (point)))
	(end-of-line)
	(narrow-to-region  anchor (point))
	(if (re-search-forward "." nil t)
	    (insert "\n")))))
  (if name
      (insert (format type name))
    (insert (format type "")))
  
  (save-excursion 
    (insert "  \"\";\n")))

(defun dynamo-insert-function (name)
  "Insert a Dynamo function declaration; use prefix argument as function name." 
  (interactive "*P")
  (if name
      (dynamo-insert-declaration "STATE FUNCTION %s" 
			       (read-string "Function name: "))
      (dynamo-insert-declaration "STATE FUNCTION %s" "")))


(defun dynamo-insert-function-table (name)
  "Insert a Dynamo function declaration; if prefix argument is given, prompt for a function name."  
  (interactive "*P")
  (if name
      (dynamo-insert-declaration "TABLE FUNCTION %s(arg) = , LOW=, HIGH=, STEP=, ARGUMENT=" 
			       (read-string "Function name: "))
      (dynamo-insert-declaration "TABLE FUNCTION %s(arg) = , LOW=, HIGH=, STEP=, ARGUMENT=" "F")))

(defun dynamo-insert-parameter (name)
  "Insert a Dynamo parameter declaration; if prefix argument is given, prompt for a parameter name."  
  (interactive "*P")
  (if name
      (dynamo-insert-declaration "PARAMETER %s" 
			       (read-string "Parameter name: "))
      (dynamo-insert-declaration "PARAMETER %s" "")))

(defun dynamo-insert-state (name)
  "Insert a Dynamo state declaration; if prefix argument is given, prompt for a parameter name." 
  (interactive "*P")
  (if name
      (dynamo-insert-declaration "STATE %s" 
			       (read-string "State name: "))
    (dynamo-insert-declaration "STATE %s" "")))


(defvar dynamo-saved-compile-command nil
  "The command last used to compile in this buffer.")

(autoload 'compile-internal "compile")

(defun dynamo-compile (command)
  "Compile a Dynamo system description file.
Runs the Dynamo compilation command, as specified in
`dynamo-compile-command', in a separate process asynchronously 
with output going to the buffer `*compilation*'. You can then use the
command \\[next-error] to find the next error message and move to the
line in the Dynamo system description file that caused it." 
  (interactive
   (list (read-string "Compile command: "
		      (or dynamo-saved-compile-command
			  (concat dynamo-compile-command
				  " "
				  (let ((name
					 (concat
					   (file-name-sans-extension
					    (buffer-file-name)) ".o")))
				    (and name
					 (file-name-nondirectory
					  name))))))))
  (setq dynamo-saved-compile-command command)
  (save-some-buffers (not compilation-ask-about-save) nil)
  (compile-internal command "No more errors"))


(define-derived-mode dynamo-mode c-mode "Dynamo"
  "Major mode for editing Dynamo system files.\n\n\\{dynamo-mode-map}"
  (c-initialize-cc-mode)
  (setq case-fold-search nil)
  (run-hooks 'c-mode-common-hook)
  (run-hooks 'dynamo-mode-hook)
  (setq font-lock-defaults '(dynamo-font-lock-keywords nil nil ((?_ . "w"))))

  (define-key dynamo-mode-map "\C-cmf" 'dynamo-insert-function)
  (define-key dynamo-mode-map "\C-cmt" 'dynamo-insert-function-table)
  (define-key dynamo-mode-map "\C-cmp" 'dynamo-insert-parameter)
  (define-key dynamo-mode-map "\C-cms" 'dynamo-insert-state)
  (define-key dynamo-mode-map "\C-cmc" 'dynamo-compile)

  (make-local-variable 'dynamo-saved-compile-command))


(setq auto-mode-alist
      (cons '("\\.dynamo$" . dynamo-mode) auto-mode-alist))


;;  $Id: dynamo-mode.el,v 1.2 2006/04/27 20:19:03 ivan_raikov Exp $
;;
;;  $Log: dynamo-mode.el,v $
;;  Revision 1.2  2006/04/27 20:19:03  ivan_raikov
;;  Added Dynamo event interface.
;;
;;  Revision 1.1  2006/02/23 01:35:35  ivan_raikov
;;  Source tree merged with RTXI.
;;
;;  Revision 1.1.1.1  2005/11/28 16:05:20  iraikov
;;  Initial
;;
;;  Revision 1.1.2.2  2005/01/23 16:42:49  iraikov
;;  Made minor changes in the insert- functions.
;;
;;  Revision 1.1.2.1  2005/01/23 16:41:40  iraikov
;;  Initial revision.
;;
;;
;;
