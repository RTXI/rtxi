;; -*-emacs-lisp-*-
(setq load-path (cons (concat "/usr/share/"
                              (symbol-name flavor)
                              "/site-lisp/dynamo") load-path))
;; Autoload dynamo-mode
(require 'dynamo-mode)
