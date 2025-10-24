#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define max(a,b) ((a>b)?a:b)
#define min(a,b) ((a>b)?b:a)

typedef size_t Sommet;
typedef size_t Lettre;

/*
    Première Partie
    Creation d'un arbre syntaxique depuis une expression rationnelle sous forme de chaîne de caractère
*/

#define SYNTAXE_OPERATOR_CONCATENATION '@' // priorité 1
#define SYNTAXE_OPERATOR_ETOILE '*' // priorité 1
#define SYNTAXE_OPERATOR_UNION '|'  // priorité 3
#define SYNTAXE_OPERATOR_SIGMA '.'
#define SYNTAXE_OPERATOR_JOKER '?' 

Lettre OPERATORS[] = {SYNTAXE_OPERATOR_ETOILE,SYNTAXE_OPERATOR_CONCATENATION,SYNTAXE_OPERATOR_UNION};

bool is_operator(Lettre l)
{
    return  l==SYNTAXE_OPERATOR_CONCATENATION ||
            l==SYNTAXE_OPERATOR_ETOILE ||
            l==SYNTAXE_OPERATOR_JOKER ||
            l==SYNTAXE_OPERATOR_UNION;
}

bool is_operator_binaire(Lettre l)
{
    return  l==SYNTAXE_OPERATOR_CONCATENATION ||
            l==SYNTAXE_OPERATOR_UNION;
}

bool is_operator_unaire(Lettre l)
{
    return  l==SYNTAXE_OPERATOR_ETOILE ||
            l==SYNTAXE_OPERATOR_JOKER ;
}

struct Tree
{
    Lettre etiquette;

    struct Tree* left_chilfren;
    struct Tree* right_children;

};

/// @brief Structure représentant un arbre syntaxique 
/// l'étiquette est un opérateur autorisé ou une lettre
/// `left_children` et `right_children` sont les enfants du noeud
/// @note le noeud n'a pas d'enfant si et seulement si `left_children=NULL`
/// @note le noeud n'a qu'un seul enfant si et seulement si  `right_children=NULL`
typedef struct Tree Tree;

Tree* Tree_init(Lettre etiquette,Tree* left,Tree* right)
{
    Tree* t = malloc(sizeof(Tree));
    t->etiquette = etiquette;
    t->left_chilfren = left;
    t->right_children = right;
    return t;
}

void Tree_free(Tree* tree)
{
    if(tree==NULL)
        return;

    if(tree->right_children!=NULL)
    {
        Tree_free(tree->right_children);
    }
    if(tree->left_chilfren!=NULL)
    {
        Tree_free(tree->left_chilfren);
    }
    free(tree);
}

/// @brief affiche l'arbre dans le terminal
/// @param tree arbre à afficher (peut-être NULL)
/// @return 
void Tree_print(Tree* tree)
{
    if(tree==NULL)
        return;

    if(tree->left_chilfren!=NULL)
    {
        printf("(");
        Tree_print(tree->left_chilfren);
        printf(")");
    }
    //printf("%c[%ld]",(char)tree->etiquette,tree->etiquette);
    printf("%c",(char)tree->etiquette);
    if(tree->right_children!=NULL)
    {
        printf("(");
        Tree_print(tree->right_children);
        printf(")");
    }

    fflush(stdout);
}

/// @brief retourne si un arbre est réduit à sa racine
/// @param tree 
/// @return 
bool is_racine(Tree* tree)
{
    return tree->left_chilfren==NULL && tree->right_children==NULL;
}


/// @brief retourne le premier Tree* non NULL dans `er` et le remplace par NULL
/// @param er 
/// @param er_size 
/// @return 
Tree* get_next_tree(Tree** er,size_t er_size)
{
    for(size_t i=0;i<er_size;i++)
    {
        if(er[i]!=NULL)
        {
            Tree* temp = er[i];
            er[i] = NULL;
            return temp;
        }
    }
    return NULL;
}

int operator_unaire_merge(Tree** er,size_t er_size,Lettre operator)
{
    int count = 0;
    Tree* last_tree = NULL;
    size_t last_tree_index = 0;
    for(size_t index=0;index<er_size;index++)
    {
        if(er[index] == NULL)
        {
            continue;
        }
        if(er[index]->etiquette == operator && is_racine(er[index]))
        {
            if(last_tree==NULL)
            {
                fprintf(stderr,"Mauvaise syntaxe dans l'utilisation de l'opérateur : %c\n",(char)operator);
                return -1;
            }

            er[index]->left_chilfren = last_tree;
            er[last_tree_index] = NULL;
            count++;

            last_tree_index = index;
            last_tree = er[index];
        }else
        {
            last_tree = er[index];
            last_tree_index = index;
        }
    }

    return count;
}

int operator_binaire_merge(Tree** er,size_t er_size,Lettre operator)
{
    int count = 0;
    Tree* last_tree = NULL;
    size_t last_tree_index = 0;
    for(size_t index=0;index<er_size;index++)
    {
        if(er[index] == NULL)
        {
            continue;
        }
        if(er[index]->etiquette == operator && is_racine(er[index]))
        {
            Tree* next_tree = get_next_tree(&er[index+1],er_size-index-1);
            if(last_tree==NULL || next_tree==NULL)
            {
                fprintf(stderr,"Mauvaise syntaxe dans l'utilisation de l'opérateur : %c\n",(char)operator);
                return -1;
            }

            er[index]->left_chilfren = last_tree;
            er[index]->right_children = next_tree;
            er[last_tree_index] = NULL;
            count++;

            last_tree_index = index;
            last_tree = er[index];
        }else
        {
            last_tree = er[index];
            last_tree_index = index;
        }
    }

    return count;
}

Tree* merge_forest(Tree** forest,size_t forest_size);

/// @brief fusionne les arbres entre la parenthèse ouvrante à l'index 0 et sa parenthèse fermante associée
/// @param regular_trees un tableau d'arbre dont le premier est réduit à une racine dont l'étiquette est '('
/// @param size la taille du tableau dans lequel la parenthèse fermante doit-être cherchée
/// @return le nombre d'arbre fusionné (et donc le nombre d'index utilisé)
size_t parentheses_merge(Tree** regular_trees,size_t size)
{
    if(regular_trees[0] == NULL || regular_trees[0]->etiquette!='(')
    {
        fprintf(stderr,"mauvaise utilisation de parentheses_merge");
        return 0;
    }

    // printf("parenthèse_merge de [");
    // for(size_t i=0;i<size;i++)
    //     if(regular_trees[i]!=NULL)
    //         printf("%c;",regular_trees[i]->etiquette);
    //     else
    //         printf("NULL;");
    // printf("]\n");

    for(size_t index = 1; index<size;index++)
    {
        if(regular_trees[index]==NULL)
            continue;

        if(regular_trees[index]->etiquette == ')')
        {
            // on a trouvé la parenthèse fermante, on libère les arbres associés a '(' ')'
            Tree_free(regular_trees[0]);
            Tree_free(regular_trees[index]);

            // reste à fusionner les arbres des index 1 à index-1 à l'aide de l'analyse des opérateur
            Tree* t = merge_forest(&regular_trees[1],index-1);
            
            // on met à NULL les arbres utilisés
            regular_trees[0]=NULL;
            regular_trees[index]=NULL;
                
            // on met l'abre issu de la fusion dans la première case de notre tableau
            regular_trees[0] = t;

            // on retourne le nombre de case utilisées
            return index;
        }
        else if(regular_trees[index]->etiquette == '(')
        {
            // on trouvé une autre parenthèse ouvrante imbiqué
            // on utilise un appel récursif pour s'occuper de celle-ci
            // on saute la zone de cette parenthèse pour continuer notre recherche après
            size_t saut = parentheses_merge(&regular_trees[index],size-index-1);
            if(saut==0)
            {
                // problème de parenthèsage
                return 0;
            }

            index+= saut;
        }else
        {
            // autres caractère on ne fait rien
        }
    }

    // aucune parenthèse fermante trouvée
    // problème
    fprintf(stderr,"Aucune parenthèse fermante trouvée, vérifiez votre usage de parenthèse_merge ou votre parenthèsage !\n");
    return 0;    
}

size_t str_len(Lettre* str)
{
    size_t size = 0;
    while (str[size]!=0)
    {
        size++;
    }
    return size;
}

Tree** make_forest(Lettre* er,size_t* forest_size)
{
    size_t er_size = str_len(er);

    // on fait une concaténation implicite entre deux lettres ou entre un opérateur unaire et une lettre à sa droite (ex :  a?a ou a*b)
    // cas des parenthèses : '(' peut-être implicitement concatener à gauche et ')' peut l'être à droite
    // si on a )( on doit faire la concaténation
    size_t nb_concatenation_implicite = 0;
    

    for(size_t i=1;i<er_size;i++)
    {
        if(er[i]=='(')
        {
            if(er[i-1]!='(' && !is_operator(er[i-1]))
                nb_concatenation_implicite++;
        }else if(!is_operator(er[i]) && er[i]!=')')
        {
            if(er[i-1]!='(' && (is_operator_unaire(er[i-1]) || !is_operator(er[i-1])))
                nb_concatenation_implicite++;
        }
    }

    *forest_size = er_size+nb_concatenation_implicite;

    Tree** forest = malloc(sizeof(Tree*)*(*forest_size));

    
    size_t current_nb_concatenation = 0;
    for(size_t i=1;i<er_size;i++)
    {
        if(er[i]=='(')
        {
            if(er[i-1]!='(' && !is_operator(er[i-1]))
            {
                forest[i+current_nb_concatenation] = Tree_init(SYNTAXE_OPERATOR_CONCATENATION,NULL,NULL);
                current_nb_concatenation++;
            }
        }else if(!is_operator(er[i]) && er[i]!=')')
        {
            if(er[i-1]!='(' && (is_operator_unaire(er[i-1]) || !is_operator(er[i-1])))
            {
                forest[i+current_nb_concatenation] = Tree_init(SYNTAXE_OPERATOR_CONCATENATION,NULL,NULL);
                current_nb_concatenation++;
            }
        }

        forest[i+current_nb_concatenation] = Tree_init(er[i],NULL,NULL);
    }
    if(er_size>0)
    forest[0] = Tree_init(er[0],NULL,NULL);

    return forest;
}

int merge_parentheses(Tree** forest,size_t forest_size)
{
    int count = 0;
    for(size_t i=0;i<forest_size;i++)
    {
       if(forest[i]!=NULL)
        {
            if(forest[i]->etiquette=='(')
            {
                if(parentheses_merge(&forest[i],forest_size-i)==0)
                {
                    return -1;
                }else
                {
                    count++;
                }
            }
        }
    }
    return count;
}

Tree* merge_forest(Tree** forest,size_t forest_size)
{
    // printf("etat initial : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");

    // fusion par parenthèse
    int error = merge_parentheses(forest,forest_size);
    if(error==-1)
        return NULL;

    // printf("après fusion des parenthèses : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");

    // gestion des ?
    error = operator_unaire_merge(forest,forest_size,SYNTAXE_OPERATOR_JOKER);
    if(error==-1)
        return NULL;

    // printf("après gestion des ? : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");

    // gestion des *
    error = operator_unaire_merge(forest,forest_size,SYNTAXE_OPERATOR_ETOILE);
    if(error==-1)
        return NULL;

    // printf("après gestion des * : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");
    
    // gestion des @
    error = operator_binaire_merge(forest,forest_size,SYNTAXE_OPERATOR_CONCATENATION);
    if(error==-1)
        return NULL;

    // printf("après gestion des @ : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");

    // gestion des |
    error = operator_binaire_merge(forest,forest_size,SYNTAXE_OPERATOR_UNION);
    if(error==-1)
        return NULL;

    // printf("après gestion des | : [");
    // for(size_t i=0;i<forest_size;i++)
    // {
    //     Tree_print(forest[i]);
    //     printf(";");
    // }
    // printf("]\n");

    Tree* t = NULL;
    for(size_t i=0;i<forest_size;i++)
    {
        if(forest[i]!=NULL)
        {
            if(t==NULL)
            {
                t=forest[i];
                forest[i]=NULL;
            }
            else
            {
                fprintf(stderr,"Erreur lors de la lecture de l'expression régulière !\n");
                return NULL;
            }
        }
    }
    
    return t;
}


Tree* make_syntaxique_tree(Lettre* er)
{
    /*
        idée : transformer le char* en une liste d'arbre
                faire un premier passage pour les parenthèses qui fusionnera certains arbres
            lancer l'analyse classique pour faire le reste
    */
    
    size_t er_size;
    Tree** trees = make_forest(er,&er_size);

    Tree* syntaxique_tree = merge_forest(trees,er_size);
    free(trees);
    return syntaxique_tree;
}


/*
    Création d'un automate à partir d'un arbre syntaxique
    Algorithme de Thomson

        le a? est équivalent à a|epsilon : automate équivalent ->()-epsilon,a->()->
        le . est équivalent à SIGMA : automate équivalent ->()-a,b,....->()->
*/

#define MIN_LISTARRAY_CAPACITY 1000
#define LISTARRAY_EXPANSION_COEF 2
struct ListArray
{
    size_t capacity;
    size_t size;
    Sommet* data;
};

/// @brief Structure de liste implémentée par tableau redimensionnable
/// @brief possibilité de faire for(size_t index=0;index<list->size;index++) list->data[index] ;;; pour parcourir la liste
/// @brief ou de faire list->data[index] pour accéder à une valeur précise en temps constant tant que index<list->size
typedef struct ListArray ListArray;

/// @brief 
/// @param  
/// @return 
ListArray* ListArray_init(void)
{
    ListArray* list = malloc(sizeof(ListArray));
    list->size = 0;
    list->capacity = MIN_LISTARRAY_CAPACITY;
    list->data = malloc(sizeof(Sommet)*list->capacity);

    return list;
}

/// @brief 
/// @param list 
void ListArray_free(ListArray* list)
{
    free(list->data);
    free(list);
}

/// @brief 
/// @param list 
/// @param s 
void ListArray_push(ListArray* list,Sommet s)
{
    if(list->size==list->capacity)
    {
#ifdef _DEBUG
        fprintf(stderr,"expansion de la liste %ld -> %ld\n",list->capacity,list->capacity*LISTARRAY_EXPANSION_COEF);
#endif
        size_t new_capacity = list->capacity * LISTARRAY_EXPANSION_COEF;
        Sommet* temp = malloc(sizeof(Sommet)*new_capacity);

        for(size_t i=0;i<list->capacity;i++)
            temp[i] = list->data[i];
        
        free(list->data);
        list->data = temp;
        list->capacity = new_capacity;

        ListArray_push(list,s);
    }else if(list->size < list->capacity)
    {
        list->data[list->size++] = s;
    }else
    {
        fprintf(stderr,"Index out of range dans une ListArray !\n");
    }
}

/// @brief 
/// @param list 
/// @return 
Sommet ListArray_pop(ListArray* list)
{
    if(list->size==0)
    {
        // problème
        fprintf(stderr,"tentative de pop sur une ListArray vide");
        return -1;
    }else
    {
        return list->data[--list->size];
    }
}

/// @brief supprime l'élément d'index `index` et le renvoie
/// @param list 
/// @param index 
/// @return 
Sommet ListArray_remove(ListArray* list,size_t index)
{
    if(index==list->size-1)
    {
        return ListArray_pop(list);
    }else
    {
        Sommet temp = list->data[index];
        list->data[index] = list->data[list->size-1];
        list->size = list->size-1;
        return temp;
    }
    

}

/// @brief 
/// @param list 
/// @return 
bool ListArray_empty(ListArray* list)
{
    return list->size==0;
}

void ListArray_print(ListArray* list)
{
    printf("[");
    for(size_t i=0;i<list->size;i++)
        printf("%ld;",list->data[i]);
    printf("]\n");
}

/// @brief instancie une copie d'une liste
/// @param list 
/// @return 
ListArray* ListArray_copy(ListArray* list)
{
    ListArray* copy = ListArray_init();
    for(size_t i=0;i<list->size;i++)
        ListArray_push(copy,list->data[i]);
    return copy;
}

/// @brief Applique une fonction à chaque valeur d'une liste
/// @param list 
/// @param f 
void ListArray_iter(ListArray* list,void (*f)(Sommet))
{
    for(size_t i=0;i<list->size;i++)
        f(list->data[i]);
}

/// @brief Applique une fonction à chaque valeur d'une liste et remplace cette valeur par le retour de la fonction
/// @param list 
/// @param f 
void ListArray_map(ListArray* list,Sommet (*f)(Sommet))
{
        for(size_t i=0;i<list->size;i++)
            list->data[i] = f(list->data[i]);
}

/// @brief concatène deux listes en une nouvelle liste
/// @param list1 
/// @param list2 
/// @return 
ListArray* ListArray_concatenation(ListArray* list1,ListArray* list2)
{
    ListArray* l = ListArray_init();
    for(size_t i=0;i<list1->size;i++)
        ListArray_push(l,list1->data[i]);
    for(size_t i=0;i<list2->size;i++)
        ListArray_push(l,list2->data[i]);

    return l;
}

/// @brief ajoute tous les éléments de `source` dans `dest`
/// @param dest liste dans laquelle les éléments vont-être ajouté (modifiée)
/// @param source liste d'ou les éléments seront récupérés (const)
void ListArray_extend(ListArray* dest,ListArray* source)
{
    for(size_t i=0;i<source->size;i++)
        ListArray_push(dest,source->data[i]);
}

#define EPSILON_TRANSITION_INDEX 0
struct Automate
{
    size_t alphabet_size;
    size_t nb_etat; // nombre d'état de l'automate
    ListArray* initiaux; // liste des états initiaux de l'automate
    ListArray* finaux; // liste des états finaux de l'automate
    ListArray*** transitions; // tableau des transitions de l'automate (une liste par état) : 
                              //j est accessible depuis i par la lettre a ssi j est dans transitions[i][a]
                              // les lettres possibles sont 1 à 255
                              // le 0 étant réservé pour les epsilon transitions
};
typedef struct Automate Automate;

Automate* Automate_init(size_t nb_etat,size_t alphabet_size)
{
    Automate* a = malloc(sizeof(Automate));
    a->alphabet_size = alphabet_size;
    a->nb_etat = nb_etat;
    a->initiaux = ListArray_init();
    a->finaux = ListArray_init();
    a->transitions = malloc(sizeof(ListArray**)*nb_etat);
    for(size_t i=0;i<nb_etat;i++)
    {
        a->transitions[i] = malloc(sizeof(ListArray*)*alphabet_size);
        for(size_t l=0;l<alphabet_size;l++)
        {
            a->transitions[i][l] = ListArray_init();
        }
    }
        

    return a;
}

void Automate_free(Automate* a)
{
    ListArray_free(a->initiaux);
    ListArray_free(a->finaux);
    for(size_t i=0;i<a->nb_etat;i++)
    {
        for(size_t l=0;l<a->alphabet_size;l++)
            ListArray_free(a->transitions[i][l]);
        free(a->transitions[i]);
    }
        

    free(a->transitions);
    free(a);
}

void Automate_print(Automate* a)
{
    if(a==NULL)return;
    printf("taille de l'aphabet : %ld\n",a->alphabet_size);
    printf("nombre d'état : %ld\n",a->nb_etat);
    printf("Etats initiaux : "); ListArray_print(a->initiaux);
    printf("Etats finaux : ");ListArray_print(a->finaux);
    for (size_t i = 0; i < a->nb_etat; i++)
    {
        printf("Depuis le sommet %ld : [",i);
        for(size_t lettre =0;lettre<a->alphabet_size;lettre++)
            for(size_t j=0;j<a->transitions[i][lettre]->size;j++)
                printf("(%c,%ld);",(char)lettre,a->transitions[i][lettre]->data[j]);
        printf("]\n");
    }
    
}

Automate* Automate_copy(Automate* a)
{
    Automate* b = Automate_init(a->nb_etat,a->alphabet_size);
    ListArray_extend(b->initiaux,a->initiaux);
    ListArray_extend(b->finaux,a->finaux);

    for(Sommet source=0;source<a->nb_etat;source++)
    {
        for(Lettre l=0;l<a->alphabet_size;l++)
        {
            ListArray_extend(b->transitions[source][l],a->transitions[source][l]);
        }
    }

    return b;
}

Sommet delta_index_to_reindexationn_fun = 0;
Sommet reindexation(Sommet s)
{
    return s + delta_index_to_reindexationn_fun;
}


void ListArray_reindexation(ListArray* list,size_t delta_index)
{
    delta_index_to_reindexationn_fun = delta_index;
    ListArray_map(list,reindexation);
    delta_index_to_reindexationn_fun = 0;
}

/// @brief réindexe les sommets d'un automate en incrémentant chacune des référence de `delta_index`
/// @param a 
/// @param delta_index 
/// @warning après l'usage de cette fonction un automate n'est plus utilisable en l'état sous peine de lecture hors mémoire
void Automate_reindexation(Automate* a,long long delta_index)
{
    delta_index_to_reindexationn_fun = delta_index;

    ListArray_map(a->initiaux,reindexation);
    ListArray_map(a->finaux,reindexation);
    for(size_t i=0;i<a->nb_etat;i++)
        for(size_t l=0;l<a->alphabet_size;l++)
            ListArray_map(a->transitions[i][l],reindexation);

    delta_index_to_reindexationn_fun = 0;
}

void Automate_add_etat_initial(Automate* a,size_t q)
{
    ListArray_push(a->initiaux,q);
}

void Automate_add_etat_final(Automate* a,size_t q)
{
    ListArray_push(a->finaux,q);
}

void Automate_add_transition(Automate* a,size_t source,size_t lettre,size_t dest)
{
    ListArray_push(a->transitions[source][lettre],dest);
}



/// @brief instancie un nouvel automate qui possède les états et transitions
/// des deux automates passés en argument (avec une réindexation de +a1->nb_etat sur a2)
/// mais ni les états finaux ni les états initiaux
/// @param a1 
/// @param a2 
/// @return 
Automate* Automate_merge(Automate* a1,Automate* a2)
{
    Automate* b = Automate_init(a1->nb_etat+a2->nb_etat,max(a1->alphabet_size,a2->alphabet_size));
    size_t delta_index = a1->nb_etat;
    // on réindexe temporairement a2
    Automate_reindexation(a2,delta_index);

    // on copie maintenant les transitions
    for(size_t etat_source=0;etat_source<a1->nb_etat;etat_source++)
    {
        for(Lettre l=0;l<a1->alphabet_size;l++)
        {
            ListArray_extend(b->transitions[etat_source][l],a1->transitions[etat_source][l]);
        }
    }

    for(size_t etat_source=0;etat_source<a2->nb_etat;etat_source++)
    {
        for(Lettre l=0;l<a2->alphabet_size;l++)
        {
            ListArray_extend(b->transitions[etat_source+delta_index][l],a2->transitions[etat_source][l]);
        }
    }
    
    
    // on répare a2
    Automate_reindexation(a2,-(long long)a1->nb_etat);

    return b;
}

/// @brief Retourne un automate reconnaissant une lettre de l'alphabet
/// @param lettre 
/// @return 
Automate* Automate_lettre(Lettre lettre,size_t alphabet_size)
{
    // ->()--lettre-->()->
    Automate* a = Automate_init(2,alphabet_size);
    Automate_add_etat_initial(a,0);
    Automate_add_etat_final(a,1);

    Automate_add_transition(a,0,lettre,1);
    return a;
}

Automate* Automate_union(Automate* a1,Automate* a2)
{
    
    Automate* b = Automate_merge(a1,a2);
    size_t delta = a1->nb_etat;

    // on copie les états finaux de a1 et de a2 vers b
    ListArray_extend(b->initiaux,a1->initiaux);
    for(size_t i=0;i<(a2->initiaux->size);i++)
        ListArray_push(b->initiaux,a2->initiaux->data[i]+delta);

    // on fait de même pour les états finaux
    ListArray_extend(b->finaux,a1->finaux);
    for(size_t i=0;i<(a2->initiaux->size);i++)
        ListArray_push(b->finaux,a2->finaux->data[i]+delta);

    //on retourne l'automate nouvellement créer
    return b;
};

Automate* Automate_concatenation(Automate* a1,Automate* a2)
{
    Automate* b = Automate_merge(a1,a2);
    // printf("Concaténation : \n");
    // Automate_print(a1);
    // Automate_print(a2);
    // printf("Après merge : \n");
    // Automate_print(b);
    size_t delta = a1->nb_etat;

    // on ajoute une epsilon transition des états finaux de a1 vers les états initiaux de a2
    for(size_t i=0;i<a1->finaux->size;i++)
    {
        Sommet source = a1->finaux->data[i];
        for(size_t j=0;j<a2->initiaux->size;j++)
        {
            Sommet dest = a2->initiaux->data[j];
            Automate_add_transition(b,source,EPSILON_TRANSITION_INDEX,dest+delta);
        }
    }

    // on met les états initiaux de a1 en tant qu'états initiaux 
    ListArray_extend(b->initiaux,a1->initiaux);
    // et les états finaux de a2 en état finaux
    ListArray_extend(b->finaux,a2->finaux);
    ListArray_reindexation(b->finaux,delta);

    // printf("fin de la concaténation : \n");
    // Automate_print(b);
    return b;
}

Automate* Automate_etoile(Automate* a)
{
    // on ajoute un état qui sera l'unique état initial et final (on va le noter q)
    // on relie tous les états initiaux de a depuis cet état
    // on relie tous les états finaux de a vers cet état

    // printf("automate etoile : entrée : ");
    // Automate_print(a);

    Automate* b = Automate_init(a->nb_etat+1,a->alphabet_size);
    Sommet q = a->nb_etat;
    ListArray_push(b->initiaux,q);
    ListArray_push(b->finaux,q);

    // on copie les transitions
    for(Sommet source =0;source<a->nb_etat;source++)
    {
        for(size_t lettre = 0;lettre<a->alphabet_size;lettre++)
        {
            ListArray_extend(b->transitions[source][lettre],a->transitions[source][lettre]);
        }
    }

    // on relie q vers les états initiaux de a
    for(size_t i=0;i<a->initiaux->size;i++)
    {
        Automate_add_transition(b,q,EPSILON_TRANSITION_INDEX,a->initiaux->data[i]);
    }

    // on relie les états finaux de a vers q
    for(size_t i=0;i<a->finaux->size;i++)
    {
        Automate_add_transition(b,a->finaux->data[i],EPSILON_TRANSITION_INDEX,q);
    }
    // printf("sortie : ");
    // Automate_print(b);
    return b;
}


Automate* Automate_joker(Automate* a)
{
    // automate reconnaissant une fois a ou rien
    // on copie a, on ajoute un état, on met cet état comme final et initial

    Automate* b = Automate_init(a->nb_etat+1,a->alphabet_size);
    ListArray_extend(b->initiaux,a->initiaux);
    ListArray_extend(b->finaux,a->finaux);
    for(Sommet s=0;s<a->nb_etat;s++)
    {
        for(Lettre l=0;l<a->alphabet_size;l++)
        {
            ListArray_extend(b->transitions[s][l],a->transitions[s][l]);
        }
    }

    ListArray_push(b->initiaux,a->nb_etat);
    ListArray_push(b->finaux,a->nb_etat);
    return b;
}

Automate* Automate_sigma(size_t alphabet_size)
{
    // automate reconnaissant tous caractère de l'alphabet et aucun
    Automate* a = Automate_init(2,alphabet_size);
    Automate_add_etat_initial(a,0);
    Automate_add_etat_final(a,1);
    for(size_t l=0;l<alphabet_size;l++)
    {
         Automate_add_transition(a,0,l,1);
    }
    return a;
}

Automate* make_thomson_automate(Tree* syntaxique_tree,size_t alphabet_size)
{
    if(syntaxique_tree==NULL)
        return NULL;

    Automate* a = NULL;
    Automate* b = NULL;
    Automate* c = NULL;
    //printf("lecture de l'abre syntaxique : "); 
    //Tree_print(syntaxique_tree);
    //printf("\n");
    switch (syntaxique_tree->etiquette)
    {
        
        case SYNTAXE_OPERATOR_CONCATENATION:
            a = make_thomson_automate(syntaxique_tree->left_chilfren,alphabet_size);
            b = make_thomson_automate(syntaxique_tree->right_children,alphabet_size);

            if(a==NULL)
            {
                if(b!=NULL)
                    Automate_free(b);
                return NULL;
            }else if(b==NULL)
            {
                if(a!=NULL)
                    Automate_free(a);
                return NULL;
            }

            c = Automate_concatenation(a,b);
            Automate_free(a);
            Automate_free(b);

            return c;
            break;
        case SYNTAXE_OPERATOR_UNION:
            a = make_thomson_automate(syntaxique_tree->left_chilfren,alphabet_size);
            b = make_thomson_automate(syntaxique_tree->right_children,alphabet_size);

            if(a!=NULL && b!=NULL)
            {
                c = Automate_union(a,b);
                Automate_free(a);
                Automate_free(b);
                return c;
            }else if(a==NULL)
            {
                return b;
            }
            if(b==NULL)
            {
                return a;
            }

            break;
        case SYNTAXE_OPERATOR_ETOILE:
            a = make_thomson_automate(syntaxique_tree->left_chilfren,alphabet_size);
            if(a==NULL) return NULL;

            b = Automate_etoile(a);
            Automate_free(a);
            return b;    
            break;
        case SYNTAXE_OPERATOR_JOKER:
            a = make_thomson_automate(syntaxique_tree->left_chilfren,alphabet_size);
            return Automate_joker(a);
            break;
        case SYNTAXE_OPERATOR_SIGMA:
            return Automate_sigma(alphabet_size);
            break;

        default:
            // lettre "normal"
            return Automate_lettre(syntaxique_tree->etiquette,alphabet_size);
            break;
    }

    return NULL;
}


/// @brief Implémentation des ensembles finies
/// Interface : initialisation O(n); libération; ajout O(n);
/// test d'appartenance O(1); fusion O(n)
struct Ensemble
{
    bool* data;
    size_t size;
};
typedef struct Ensemble Ensemble;


ListArray* ensemble_pool = NULL;
void Ensemble_init_pool(void)
{
    if(ensemble_pool==NULL)
    {
        ensemble_pool = ListArray_init();
    }
}

void Ensemble_free_pool(void)
{
    if(ensemble_pool!=NULL)
    {
        for(size_t i=0;i<ensemble_pool->size;i++)
        {
            Ensemble* e = (Ensemble*)ensemble_pool->data[i];
            free(e->data);
            free(e);
        }
        ListArray_free(ensemble_pool);
    }
}

/// @brief Instancie un ensemble dont le nombre d'élément ne dépassera pas `n`
/// l'ensemble est initialement vide
/// @param n 
/// @return un pointeur vers un ensemble
Ensemble* Ensemble_init(size_t n)
{   
    if(ensemble_pool==NULL)
    {
       Ensemble_init_pool();
    }

    {
        Ensemble* e = NULL;
        for(size_t i=0;i<ensemble_pool->size;i++)
        {
            Ensemble* current = (Ensemble*)ensemble_pool->data[i];// on utilise l'abut sizeof(Ensemble*)=sizeof(size_t)=sizeof(Sommet) (c'est à dire 64 bits)
            if(current->size==n)
            {
                e = (Ensemble*)ListArray_remove(ensemble_pool,i);
                break;
            }
        }
        if(e==NULL) // pas d'ensemble à la bonne taille
        {
            e = malloc(sizeof(Ensemble));
            e->size = n;
            e->data = malloc(sizeof(bool)*n);
        }
        
        for(size_t i=0;i<n;i++)
            e->data[i]=false;
        return e;
        
    }


}

void Ensemble_free(Ensemble* e)
{
    if(ensemble_pool==NULL)
    {
        Ensemble_init_pool();
    }

    ListArray_push(ensemble_pool,(Sommet)e);
}

void Ensemble_print(Ensemble* e)
{
    printf("{");
    for(size_t i=0;i<e->size;i++)
        if(e->data[i])
            printf("%ld;",i);
    printf("}\n");
}

/// @brief Ajoute un élément à un ensemble
/// @param e 
/// @param s 
void Ensemble_add(Ensemble* e,Sommet s)
{
    e->data[s] = true;
}

/// @brief teste si un élément est dans un ensemble
/// @param e 
/// @param s 
/// @return true si `s` est dans `e`
bool Ensemble_mem(Ensemble* e,Sommet s)
{
    return e->data[s];
}

/// @brief Instancie une copie d'un ensemble
/// @param e 
/// @return 
Ensemble* Ensemble_copy(Ensemble* e)
{
    Ensemble* new_e = Ensemble_init(e->size);
    memcpy(new_e->data,e->data,e->size*sizeof(bool));
    return new_e;
}

/// @brief Instancie un nouvel enemble contenant tous les éléments de a et de b
/// @param a 
/// @param b 
/// @return 
Ensemble* Ensemble_merge(Ensemble* a,Ensemble* b)
{
    Ensemble* c = Ensemble_init(a->size);
    for(size_t i=0;i<c->size;i++)
    {
        c->data[i] = (Ensemble_mem(a,i))||(Ensemble_mem(b,i));
    }
    return c;
}

/// @brief retourne si un ensemble est vide
/// @param e
/// @return true si l'ensemble est vide, false sinon
bool Ensemble_vide(Ensemble* e)
{
    for(size_t i=0;i<e->size;i++)
        if(e->data[i])
            return false;
    return true;
}

/// @brief insère tous les éléments d'une liste dans un ensemble
/// @param e 
/// @param list 
void Ensemble_eat_list(Ensemble* e,ListArray* list)
{
    for(size_t i=0;i<list->size;i++)
        Ensemble_add(e,list->data[i]);
}

void _rec_cloture_etat_depth_search(Automate* a,Sommet q,Ensemble* vus)
{
    if(Ensemble_mem(vus,q))
    {
        return;
    }else
    {
        Ensemble_add(vus,q);
        for(size_t i=0;i<a->transitions[q][EPSILON_TRANSITION_INDEX]->size;i++)
            _rec_cloture_etat_depth_search(a,a->transitions[q][EPSILON_TRANSITION_INDEX]->data[i],vus);
    }
}

/// @brief retoune la cloture instantanée d'un état dans un automate à epsilon transition
/// @param a 
/// @param q 
/// @return 
Ensemble* Automate_cloture_instantanee_etat(Automate* a,Sommet q)
{
    Ensemble* cloture = Ensemble_init(a->nb_etat);
    _rec_cloture_etat_depth_search(a,q,cloture);
    return cloture;
}

/// @brief Calcule l'union des clotures instantannées des états de `e`
/// @param a 
/// @param e 
/// @return 
Ensemble* Automate_cloture_instantanee(Automate* a,Ensemble* e)
{
    Ensemble* Q = Ensemble_copy(e);
    for(size_t i=0;i<a->nb_etat;i++)
    {
        if(Ensemble_mem(e,i))
        {
            for(size_t j=0;j<a->transitions[i][EPSILON_TRANSITION_INDEX]->size;j++)
            {
                _rec_cloture_etat_depth_search(a,a->transitions[i][EPSILON_TRANSITION_INDEX]->data[j],Q);
            }
        }
    }

    return Q;
}

/// @brief même chose que `Automate_cloture_instantanee` mais avec liberation/consommation de `e`
/// @param a 
/// @param e 
/// @return 
Ensemble* Automate_cloture_instantanee_inplace(Automate* a,Ensemble* e)
{
    Ensemble* temp = Automate_cloture_instantanee(a,e);
    Ensemble_free(e);
    return temp;
}


/// @brief retourne l'ensemble des états accessibles dans l'automate `a`
/// depuis les états de `e` en lisant la lettre l
/// @warning les epsilon transition ne sont pas considérées
/// @param a 
/// @param e 
/// @param l 
/// @return 
Ensemble* Automate_read_letter(Automate* a,Ensemble* e,size_t l)
{
    if(l>a->alphabet_size)
    {
        fprintf(stderr,"Impossible de lire la lettre %c(%ld) avec un automate d'alphabet de taille %ld\n",(char)l,l,a->alphabet_size);
        return Ensemble_init(a->nb_etat);
    }

    Ensemble* dest = Ensemble_init(a->nb_etat);
    
    for (size_t i = 0; i < a->nb_etat; i++)
    {
        if(Ensemble_mem(e,i))
        {
            Ensemble_eat_list(dest,a->transitions[i][l]);
        }
    }
    
    return dest;
}

/// @brief détermine si il y a un état final dans un ensemble
/// @param a 
/// @param e 
/// @return 
bool Automate_is_final_ensemble(Automate* a,Ensemble* e)
{
    for(size_t i=0;i<a->finaux->size;i++)
    {
        if(Ensemble_mem(e,a->finaux->data[i]))
            return true;
    }
    return false;
}

/// @brief détermine si un mot est reconnu par un automate
/// @param a 
/// @param e 
/// @param word 
/// @return 
bool Automate_read_word(Automate* a,Lettre* word)
{
    // on initialise un ensemble avec les états initiaux et les états dans la cloture instantanée des états initiaux
    Ensemble* initiaux = Ensemble_init(a->nb_etat);
    Ensemble_eat_list(initiaux,a->initiaux);
    Ensemble* temp = Automate_cloture_instantanee(a,initiaux);
    Ensemble_free(initiaux);
    initiaux = temp;

    //printf("début de la lecture du mot %s, états de départ : ",word);
    //Ensemble_print(initiaux);

    // on lit ensuite chaque lettre de manière itérative
    // sans oublie de calculer la cloture instantanée à chaque fois
    for(size_t index=0;word[index]!=0;index++)
    {
        temp = Automate_read_letter(a,initiaux,word[index]);
        //printf("Après lecture de la lettre '%c'\n",word[index]);
        //Ensemble_print(temp);

        Ensemble_free(initiaux);
        initiaux = Automate_cloture_instantanee(a,temp);
        Ensemble_free(temp);
        //printf("Après cloture instantanée\n");
        //Ensemble_print(initiaux);
    }

    // on regarde si il y a un état final dans les états obtenus
    bool is_final = Automate_is_final_ensemble(a,initiaux);
    Ensemble_free(initiaux);

    return is_final;
}

/// @brief retourne un automate, inverse le sens de ses transitions et inverse initiaux et finaux
/// @param a 
/// @return un nouvel automate
Automate* Automate_reverse(Automate* a)
{
    Automate* b = Automate_init(a->nb_etat,a->alphabet_size);
    
    // on inverse initiaux et finaux
    ListArray_extend(b->initiaux,a->finaux);
    ListArray_extend(b->finaux,a->initiaux);

    // on inverse les transitions
    for(Sommet source=0;source<a->nb_etat;source++)
    {
        for(Lettre l=0;l<a->alphabet_size;l++)
        {
            for(size_t i=0;i<a->transitions[source][l]->size;i++)
            {
                Sommet dest = a->transitions[source][l]->data[i];
                Automate_add_transition(b,dest,l,source);
            }
        }
    }

    return b;
}


Automate* Automate_line(Automate* a)
{
    // on construit l'automate reconnaissant ".e" avec e l'expression régulière dont le langage est dénoté par a
    // c'est à dire "(.)*e" 
    Automate* b = Automate_sigma(a->alphabet_size);
    Automate* temp = Automate_concatenation(b,a);
    Automate_free(b);
    return temp;
}

/// @brief lit une chaîne de caractère et détecte les motifs reconnu par l'automate `a`
/// et renvoie une liste des index de fin de ces motifs dans la chaîne `line`
/// @note on privilégiera toujours les motifs les plus petits : 
/// exemple avec le texte "abab" et l'automate reconnaissant "a(a|b)*b", "abab" est vu comme deux motifs
/// @warning find("ab*a","aabbbaba") -> aa et aba donc [1;7]
/// @param a 
/// @param line 
/// @return 
ListArray* find_motif_end_indexs(Automate* line_automate,Lettre* line)
{
    Ensemble* Q = Ensemble_init(line_automate->nb_etat);
    Ensemble_eat_list(Q,line_automate->initiaux);
    Q = Automate_cloture_instantanee_inplace(line_automate,Q);
    Ensemble* Q_init = Ensemble_copy(Q);

    ListArray* indexs = ListArray_init();
    size_t current_index = 0;

    //printf("avant lecture : \n");
    //Ensemble_print(Q);

    while (line[current_index]!='\0')// tant que toutes la chaîne n'a pas été lue
    {
        Lettre l = line[current_index];
        Ensemble* next_Q = Automate_cloture_instantanee_inplace(line_automate,Automate_read_letter(line_automate,Q,l));
        bool next_Q_final = Automate_is_final_ensemble(line_automate,next_Q);

        //printf("lecture de la lettre %c, etat_final=%d, Q = ",(char)l,next_Q_final);Ensemble_print(next_Q);


        if(next_Q_final)
        {
            // current_index est donc le dernier carectère d'un motif reconnu
            ListArray_push(indexs,current_index);
            Ensemble_free(next_Q);
            next_Q = Ensemble_copy(Q_init);
        }


        current_index++;
        Ensemble_free(Q);
        Q = next_Q;
    }

    Ensemble_free(Q);
    Ensemble_free(Q_init);
    //printf("retour des index de fins : ");ListArray_print(indexs);printf("\n");
    return indexs;
}

/// @brief Retrouve les index de début des motifs reconnu par `a` dans `line`
/// à partir des index de fin de ces même motifs, (obtenu précedemment avec `find_motif_end_indexs`)
/// @param a 
/// @param line 
/// @param end_indexs 
/// @return 
ListArray* find_motif_start_indexs(Automate* reverse_automate,Lettre* line,ListArray* end_indexs)
{
    ListArray* indexs = ListArray_init();

    for(size_t i=0;i<end_indexs->size;i++)
    {
        size_t end = end_indexs->data[i];
        Ensemble* Q = Ensemble_init(reverse_automate->nb_etat);
        Ensemble_eat_list(Q,reverse_automate->initiaux);
        Q = Automate_cloture_instantanee_inplace(reverse_automate,Q);

        size_t current_index = end;
        while (!Automate_is_final_ensemble(reverse_automate,Q))
        {
            Lettre l = line[current_index];
            Ensemble* next_Q = Automate_cloture_instantanee_inplace(reverse_automate,Automate_read_letter(reverse_automate,Q,l));
            Ensemble_free(Q);
            Q = next_Q;
            current_index--;
        }
        ListArray_push(indexs,current_index+((current_index==end)?0:1)); 
        Ensemble_free(Q);
    }
    return indexs;
}

/// @brief lit une ligne d'un flux
/// @param flux 
/// @return 
Lettre* get_line(FILE* flux)
{
    ListArray* list = ListArray_init();
    int l = getc(flux);
    if(l<=0)goto EOF_HAPPEND;
    if(l=='\n' && flux==stdin)goto EOF_HAPPEND;

    while (l!='\n' && l>0)
    {
        ListArray_push(list,l);
        l = getc(flux);
    }
    ListArray_push(list,0);

    //printf("lecture de %ld octets : ",list->size);ListArray_print(list);printf("\n");
    Lettre* line = malloc(sizeof(Lettre)*list->size);
    for(size_t i=0;i<list->size;i++)
        line[i] = list->data[i];
    
    ListArray_free(list);
    return line;

    EOF_HAPPEND:
    ListArray_free(list);
    return NULL;
}

enum COLOR
{
    WHITE,
    RED
};

void set_stdout_color(enum COLOR color)
{
#ifdef __linux
    switch (color)
    {
    case WHITE:
        printf("\e[0;37m");
        break;
    case RED:
        printf("\e[0;31m");
        break;
    default:
        break;
    }
#endif


#ifdef _WIN32
    HANDLE  hConsole;
    int k;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


    switch (color)
    {
    case WHITE:
        SetConsoleTextAttribute(hConsole, 15);
        break;
    case RED:
        SetConsoleTextAttribute(hConsole, 12);
        break;
    default:
        break;
    }

    //// you can loop k higher to see more color choices
    //for (k = 1; k < 255; k++)
    //{
    //    SetConsoleTextAttribute(hConsole, k);
    //    printf("%3d  %s\n", k, "I want to be nice today!");     
    //}
#endif 

}

void afficher_motifs(Lettre* line,ListArray* starts,ListArray* ends)
{
    size_t current_index = 0;
    for(size_t i =0;i<starts->size;i++)
    {
        size_t start = starts->data[i];
        size_t end = ends->data[i];
    
        while (current_index<start)
        {
            putc(line[current_index++],stdout);
        }
        
        set_stdout_color(RED);
        while (current_index<=end)
        {
            putc(line[current_index++],stdout);
        }
        set_stdout_color(WHITE);
    }

    while (line[current_index]!='\0')
    {
        putc(line[current_index++],stdout);
    }
    putc('\n',stdout);
}

Lettre* translate(char* sentence,size_t alphabet_size)
{
    size_t size = strlen(sentence);
    Lettre* line = malloc(sizeof(Lettre)*(size+1));
    for(size_t i=0;i<size;i++)
        line[i] = sentence[i];
    line[size] = 0;
    return line;
}

int main(int argc,char** argv)
{
    bool extended_expression = false;
    char* input_filename = NULL;
    char* regular_expression_char = NULL;
    FILE* source = NULL;
    size_t alphabet_size = 255;
    bool verbose = false;
    bool show_line = false;

    for(size_t i=1;i<argc;i++)
    {
        char* arg = argv[i];

        if(strcmp(arg,"-E")==0)
        {
            extended_expression = true;
        }else if(strcmp(arg,"--alphabet")==0 || strcmp(arg,"-A")==0)
        {
            alphabet_size = atoll(argv[++i]);
        }else if(strcmp(arg,"--verbose")==0)
        {
            verbose= true;
        }else if(strcmp(arg,"--line")==0)
        {
            show_line = true;
        }
        else
        {
            if(regular_expression_char==NULL)
                regular_expression_char = arg;
            else
                input_filename = arg;
        }
    }

    if(regular_expression_char==NULL)
    {
        fprintf(stderr,"Argument maquant !\n");
        return 1;
    }


    Lettre* regular_expression = translate(regular_expression_char,alphabet_size);

    if(verbose)
    {
        fprintf(stderr,"Recherche du motif \'%s\' dans ",regular_expression_char);
        if(input_filename!=NULL)
        {
            fprintf(stderr,"le fichier %s \n",input_filename);
        }else
        {
            fprintf(stderr,"l'entrée standard\n");
        }
    }
    
    Tree* t = NULL;
    Automate* a = NULL;
    
    if(extended_expression)
    {
        t = make_syntaxique_tree(regular_expression);
        if(verbose)
        {
            printf("arbre syntaxique : ");Tree_print(t); printf("\n");
        }
         
        a = make_thomson_automate(t,alphabet_size);
        if(verbose)
        {
            Automate_print(a);
        }

    }else
    {
        fprintf(stderr,"non implémenté\n");
        return 1;
    }

    if(a==NULL)
    {
        fprintf(stderr,"Impossible de construire l'automate associé à l'expression %s !\n",regular_expression_char);
        return 1;
    }

    if(input_filename==NULL)
    {
        source = stdin;
    }else
    {

        source = fopen(input_filename,"r");
        if (source==NULL)
        {
            fprintf(stderr,"Impossible d'ouvrir le fichier %s!\n",input_filename);
            return 1;
        }
        
    }

    Automate* reverse_automate = Automate_reverse(a);
    Automate* line_automate = Automate_line(a);

    size_t line_count = 0;
    Lettre* line = NULL;
    while ((line=get_line(source))!=NULL)
    {
        if(verbose && source!=stdin)
        {
            printf("line %ld\r",line_count);
        }
        //printf("lecture de \"%s\"\n",line);
        ListArray* ends = find_motif_end_indexs(line_automate,line);
        ListArray* starts = find_motif_start_indexs(reverse_automate,line,ends);

        if(starts->size>0)
        {
            if(show_line)
                printf("%ld : ",line_count);
            if(verbose)
                printf(" %ld motifs : ",ends->size);
            afficher_motifs(line,starts,ends);
        }

        ListArray_free(starts);
        ListArray_free(ends);
        free(line);
        line_count++;
    }
    

    if(a!=NULL)
    {
        Automate_free(a);
        Automate_free(reverse_automate);
        Automate_free(line_automate);
    }
    if(t!=NULL)Tree_free(t);
    if(regular_expression!=NULL)free(regular_expression);
    Ensemble_free_pool();
    return 0;
}



/*

time ./mygrep  -E "(.*q.*w.*)|(.*w.*q.*)" Donnees_grep/francais.txt 
clownesque
clownesques
squaw
squaws
wisigothique
wisigothiques

real    0m0.279s
user    0m0.184s
sys     0m0.023s


time grep -E "(^.*q.*w.*$)|(^.*w.*q.*$)" Donnees_grep/francais.txt 
clownesque
clownesques
squaw
squaws
wisigothique
wisigothiques

real    0m0.015s
user    0m0.002s
sys     0m0.002s
*/