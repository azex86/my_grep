# Plan d'attaque
## 1 interpreter les arguments de la commande
>paramètres :  mygrep [-vh] [optionnal args] [filename]
    verbose  -v
    help : --help -h
    expression regulière étendue : -E <er>
    
## 2 intertreter l'expression rationnelle en un arbre syntaxique
on souhaite tranqformer une expression régulière de la forme (a*@b)|(b@a) en un arbre
$$
\documentclass{article} 
\usepackage{tikz} 
\begin{document} 
\begin{tikzpicture}[main/.style = {draw, circle}] 
\node[main] (1) {$x_1$}; 
\end{tikzpicture} 
\end{document}
$$      
## 3 construire un automate associé à l'expression rationnel par Berry-Setty ou Thomson
## 4 lire le fichier en appliquant l'automate à chaque ligne