#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef size_t Sommet;


/*
    Première Partie
    Creation d'un arbre syntaxique depuis une expression rationnelle sous forme de chaîne de caractère
*/

#define SYNTAXE_OPERATOR_CONCATENATION '@' // priorité 1
#define SYNTAXE_OPERATOR_ETOILE '*' // priorité 0
#define SYNTAXE_OPERATOR_UNION '|'  // priorité 2
#define SYNTAXE_OPERATOR_SIGMA '?'
#define SYNTAXE_OPERATOR_JOKER '.'

char OPERATORS[] = {SYNTAXE_OPERATOR_ETOILE,SYNTAXE_OPERATOR_CONCATENATION,SYNTAXE_OPERATOR_UNION};

struct Tree
{
    char etiquette;

    struct Tree* left_chilfren;
    struct Tree* right_children;

};

/// @brief Structure représentant un arbre syntaxique 
/// l'étiquette est un opérateur autorisé ou une lettre
/// `left_children` et `right_children` sont les enfants du noeud
/// @note si et seulement si le noeud n'a pas d'enfant `left_children=NULL`
/// @note si et seulement si le noeud n'a qu'un seul enfant `right_children=NULL`
typedef struct Tree Tree;

Tree* Tree_init(char etiquette,Tree* left,Tree* right)
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
    printf("%c",tree->etiquette);
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

Tree* char_slice(Tree** er,size_t er_size)
{
    // on veut maintenant séparer er[0:er_size] en caractères
    // avec une concaténation entre chaque
    if(er_size>0)
    {
        
        if(er[0]!=NULL)
        {
            Tree* next = char_slice(&er[1],er_size-1);
            Tree* t = NULL;
            if(next!=NULL)
            {
                // on concatène le caractère actuel avec next
                t = Tree_init(SYNTAXE_OPERATOR_CONCATENATION,er[0],NULL);
                t->right_children = next;
            }else
            {
                // on a trouvé le dernier caractère, on le renvoit
                t = er[0];
            }
            
            er[0] = NULL;
            return t;
        }
        else
        {
            return char_slice(&er[1],er_size-1);
        }
    }

    return NULL;
}

/// @brief construit l'arbre syntaxique d'une expression régulière en considérant 
/// que l'opérateur fournit est celui avec la plus haute priorité et les blocks sont ensuite arbrifié caractère par caractère
/// @param er un tableau d'arbre syntaxique, les cases utilisé sont mises à NULL
/// @param er_size la taille de la chaine (utilse si on a veut rechercher dans une partie de la châine)
/// @param  oprator priorité de l'opérateur cherché
/// @return un arbre syntaxique
/// @example operator_binaire_slice("a|a@a",5,2) -> un arbre dont l'affichage est : ((a)|(a))@(a)
Tree* operator_unaire_slice(Tree** er,size_t er_size,int operator_priority)
{
    char operator = OPERATORS[operator_priority];

    //printf("Recherche de l'opérateur %c de priorité %d sur ",operator,operator_priority);
    //for(int i=0;i<er_size;i++)
    //    if(er[i]!=NULL)
    //    {
    //         if(is_racine(er[i]))
    //             printf("%c",er[i]->etiquette);
    //         else
    //             printf("(%c)",er[i]->etiquette);
    //     }
        
    // printf("\n");


    size_t index=0;
    for(;index<er_size;index++)
    {
        if(er[index]==NULL)
            continue;

        if(er[index]->etiquette==operator && is_racine(er[index]))
        {
            er[index]->left_chilfren = char_slice(er,index);
            er[index]->right_children = NULL;

            Tree* t = er[index];
            er[index] = NULL;
            return t;
        }
    }

    return  char_slice(er,er_size);
}

/// @brief construit l'arbre syntaxique d'une expression régulière en considérant 
/// que l'opérateur fournit est celui avec la plus haute priorité et les blocks sont ensuite 
/// appelé récursivement sur l'opérateur suivant donné par OPERATOR_PRIORITY
/// @warning les arbres du tableau sont ajouté à l'arbre construit, ils ne doivent donc pas être libérés
/// @param er le tableau d'arbre dans laquelle on doit procéder, les cases utilisées sont mises à NULL
/// @param er_size la taille de la chaine (utilse si on a veut rechercher dans une partie de la châine)
/// @param  operator_priority priorité de l'opérateur cherché, 2 pour lancer l'analyse depuis le début
/// @return un arbre syntaxique
/// @example operator_binaire_slice("a|a@a",5,2) -> un arbre dont l'affichage est : ((a)|(a))@(a)
Tree* operator_binaire_slice(Tree** er,size_t er_size,int operator_priority)
{
    if(operator_priority==0)
    {
        return operator_unaire_slice(er,er_size,0);
    }

    char operator = OPERATORS[operator_priority];

    // printf("Recherche de l'opérateur %c de priorité %d sur ",operator,operator_priority);
    // for(int i=0;i<er_size;i++)
    //     if(er[i]!=NULL)
    //     {
    //         if(is_racine(er[i]))
    //             printf("%c",er[i]->etiquette);
    //         else
    //             printf("(%c)",er[i]->etiquette);
    //     }

    // printf("\n");

    Tree* current_tree = NULL;
    size_t last_index = 0;
    for(size_t index=0;index<er_size;index++)
    {
        if(er[index]==NULL)continue;

        if(er[index]->etiquette==operator && is_racine(er[index]))
        {
            if(current_tree==NULL)
            {
                // on arrive sur le premier operateur ce cette sorte
                er[index]->left_chilfren = operator_binaire_slice(&er[last_index],index-last_index,operator_priority-1);
                er[index]->right_children = NULL;
                current_tree = er[index];
                er[index] = NULL;
            }else
            {
                current_tree->right_children = operator_binaire_slice(&er[last_index],index-last_index,operator_priority-1);
                er[index]->left_chilfren = current_tree;
                current_tree = er[index];
                er[index] = NULL;
            }
            last_index = index+1;
        }
    }
    if(current_tree!=NULL && current_tree->right_children==NULL)
    {
        current_tree->right_children = operator_binaire_slice(&er[last_index],er_size-last_index,operator_priority-1);
    }
    else if(current_tree==NULL)
    {
        // aucun operateur n'a été trouvé on passe tout sur l'opérateur suivant
        return operator_binaire_slice(er,er_size,operator_priority-1);
    }
    return current_tree;
}




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
            Tree* t = operator_binaire_slice(&regular_trees[1],index-1,2);
            
            // on met à NULL les arbres utilisés
            for(size_t j=0;j<=index;j++)
                regular_trees[j] = NULL;
                
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

Tree* make_syntaxique_tree(char* er)
{
    /*
        idée : transformer le char* en une liste d'arbre
                faire un premier passage pour les parenthèses qui fusionnera certains arbres
            lancer l'analyse classique pour faire le reste
    */
    size_t er_size = strlen(er);
    Tree** trees = malloc(sizeof(Tree*)*er_size);

    // transformation de ma chaîne de caractère en arbres 
    for(size_t i=0;i<er_size;i++)
    {
        trees[i] = Tree_init(er[i],NULL,NULL);
    }

    // fusion par parenthèse
    for(size_t i=0;i<er_size;i++)
    {
       if(trees[i]!=NULL)
            if(trees[i]->etiquette=='(')
                if(parentheses_merge(&trees[i],er_size-i)==0)
                {
                    return NULL;
                }
    }

    printf("Après fusion des parenthèses : [");
    for(size_t i=0;i<er_size;i++)
    {
        Tree_print(trees[i]);
        printf(";");
    }
    printf("]\n");

    // application des fusions des opérateurs
    Tree* t = operator_binaire_slice(trees,er_size,2);

    free(trees);
    return t;
}


/*
    Création d'un automate à partir d'un arbre syntaxique
    Algorithme de Thomson

        le ? est équivalent à SIGMA : automate équivalent ->()-a,b,...->()->
        le . est équivalent à SIGMA* : automate équivalent ->(())<-a,b,....>
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
        return list->data[list->size--];
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

#define ALPHABET_SIZE (unsigned char)(-1)
#define EPSILON_TRANSITION_INDEX 0
struct Automate
{
    size_t nb_etat; // nombre d'état de l'automate
    ListArray* initiaux; // liste des états initiaux de l'automate
    ListArray* finaux; // liste des états finaux de l'automate
    ListArray*** transitions; // tableau des transitions de l'automate (une liste par état) : 
                              //j est accessible depuis i par la lettre a ssi j est dans transitions[i][a]
                              // les lettres possibles sont 1 à 255
                              // le 0 étant réservé pour les epsilon transitions
};
typedef struct Automate Automate;

Automate* Automate_init(size_t nb_etat)
{
    Automate* a = malloc(sizeof(Automate));
    a->nb_etat = nb_etat;
    a->initiaux = ListArray_init();
    a->finaux = ListArray_init();
    a->transitions = malloc(sizeof(ListArray**)*nb_etat);
    for(size_t i=0;i<nb_etat;i++)
    {
        a->transitions[i] = malloc(sizeof(ListArray*)*ALPHABET_SIZE);
        for(size_t l=0;l<ALPHABET_SIZE;l++)
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
        for(size_t l=0;l<ALPHABET_SIZE;l++)
            ListArray_free(a->transitions[i][l]);
        free(a->transitions[i]);
    }
        

    free(a->transitions);
    free(a);
}

void Automate_print(Automate* a)
{
    if(a==NULL)return;
    printf("nombre d'état : %ld\n",a->nb_etat);
    printf("Etats initiaux : "); ListArray_print(a->initiaux);
    printf("Etats finaux : ");ListArray_print(a->finaux);
    for (size_t i = 0; i < a->nb_etat; i++)
    {
        printf("Depuise le sommet %ld : [",i);
        for(size_t lettre =0;lettre<ALPHABET_SIZE;lettre++)
            for(size_t j=0;j<a->transitions[i][lettre]->size;j++)
                printf("(%c,%ld);",(char)lettre,a->transitions[i][lettre]->data[j]);
        printf("]\n");
    }
    
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
void Automate_reindexation(Automate* a,size_t delta_index)
{
    delta_index_to_reindexationn_fun = delta_index;

    ListArray_map(a->initiaux,reindexation);
    ListArray_map(a->finaux,reindexation);
    for(size_t i=0;i<a->nb_etat;i++)
        for(size_t l=0;l<ALPHABET_SIZE;l++)
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


/// @brief instancie une version normalisée d'un automate avec un seul état initial et un seul état final
/// @param a 
/// @return 
Automate* Automate_normalisation(Automate* a)
{
    return NULL;
}

/// @brief instancie un nouvel automate qui possède les états et transitions
/// des deux automates passés en argument (avec une réindexation de +a1->nb_etat sur a2)
/// mais ni les états finaux ni les états initiaux
/// @param a1 
/// @param a2 
/// @return 
Automate* Automate_merge(Automate* a1,Automate* a2)
{
    Automate* b = Automate_init(a1->nb_etat+a2->nb_etat);
    size_t delta_index = a1->nb_etat;
    // on réindexe temporairement a2
    Automate_reindexation(a2,delta_index);

    // on copie maintenant les transitions
    for(size_t etat_source=0;etat_source<a1->nb_etat;etat_source++)
    {
        for(size_t l=0;l<ALPHABET_SIZE;l++)
        {
            ListArray_extend(b->transitions[etat_source][l],a1->transitions[etat_source][l]);
        }
    }

    for(size_t etat_source=0;etat_source<a2->nb_etat;etat_source++)
    {
        for(size_t l=0;l<ALPHABET_SIZE;l++)
        {
            ListArray_extend(b->transitions[etat_source+delta_index][l],a2->transitions[etat_source][l]);
        }
    }
    
    
    // on répare a2
    Automate_reindexation(a2,-a1->nb_etat);

    return b;
}

/// @brief Retourne un automate reconnaissant une lettre de l'alphabet
/// @param lettre 
/// @return 
Automate* Automate_lettre(size_t lettre)
{
    // ->()--lettre-->()->
    Automate* a = Automate_init(2);
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
            Sommet dest = a2->finaux->data[j];
            Automate_add_transition(b,source,EPSILON_TRANSITION_INDEX,dest);
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

    Automate* b = Automate_init(a->nb_etat+1);
    Sommet q = a->nb_etat;
    ListArray_push(b->initiaux,q);
    ListArray_push(b->finaux,q);

    // on copie les transitions
    for(Sommet source =0;source<a->nb_etat;source++)
    {
        for(size_t lettre = 0;lettre<ALPHABET_SIZE;lettre++)
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

    return b;
}

Automate* make_thomson_automate(Tree* syntaxique_tree)
{
    if(syntaxique_tree==NULL)
        return NULL;

    Automate* a = NULL;
    Automate* b = NULL;
    Automate* c = NULL;

    switch (syntaxique_tree->etiquette)
    {
        case SYNTAXE_OPERATOR_CONCATENATION:
            a = make_thomson_automate(syntaxique_tree->left_chilfren);
            b = make_thomson_automate(syntaxique_tree->right_children);

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
            a = make_thomson_automate(syntaxique_tree->left_chilfren);
            b = make_thomson_automate(syntaxique_tree->right_children);

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
            a = make_thomson_automate(syntaxique_tree->left_chilfren);
            if(a==NULL) return NULL;

            b = Automate_etoile(a);
            Automate_free(a);
            return b;    
            break;
        case SYNTAXE_OPERATOR_JOKER:
            break;
        case SYNTAXE_OPERATOR_SIGMA:

            break;

        default:
            // lettre "normal"
            return Automate_lettre(syntaxique_tree->etiquette);
            break;
    }

    return NULL;
}


enum SOURCE
{ 
    SOURCE_FILE,
    SOURCE_STDIN,
};

int main(int argc,char** argv)
{
    char* input_filename = NULL;
    char* regular_expression = NULL;
    enum SOURCE source = SOURCE_STDIN;

    for(size_t i=1;i<argc;i++)
    {
        char* arg = argv[i];

        if(strcmp(arg,"-E")==0)
        {
            regular_expression = argv[++i];
        }else
        {
            source = SOURCE_FILE;
            input_filename = arg;
        }
    }

    char* reg = "(a)*";//"(a|b)*ab(a|b)*";
    if(regular_expression==NULL)
        regular_expression = reg;

    printf("Recherche du motif \'%s\' dans ",regular_expression);
    if(source==SOURCE_FILE)
    {
        printf("le fichier %s \n",input_filename);
    }else
    {
        printf("l'entrée standard\n");
    }

    Tree* t = make_syntaxique_tree(regular_expression);
    Tree_print(t);
    printf("\n");

    Automate* a = make_thomson_automate(t);
    if(a==NULL)
        printf("Impossible de construire l'automate associé à l'expression %s !\n",regular_expression);
    Automate_print(a);


    if(a!=NULL)Automate_free(a);
    if(t!=NULL)Tree_free(t);

    return 0;
}