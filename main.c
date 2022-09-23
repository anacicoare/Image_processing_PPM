#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct QuadtreeNode {

    unsigned char blue;
    unsigned char green;
    unsigned char red;

    uint32_t area;

    uint32_t top_left;
    uint32_t top_right;
    uint32_t bottom_left;
    uint32_t bottom_right;

} __attribute__((packed)) QuadtreeNode;

typedef struct pixels {

    uint8_t blue;
    uint8_t green;
    uint8_t red;

} __attribute__((packed)) pixels_struct;

typedef struct Quadtree {

    QuadtreeNode data;
    uint32_t index;

    struct Quadtree* parent;

    struct Quadtree* top_left;
    struct Quadtree* top_right;
    struct Quadtree* bottom_right;
    struct Quadtree* bottom_left;

} __attribute__((packed)) Quadtree;

typedef struct queue_node {

    Quadtree* data;
    struct queue_node* next;
    struct queue_node* prev;

} __attribute__((packed)) queue_node;
typedef struct queue_struct {

    uint32_t len;
    struct queue_node* head;
    struct queue_node* tail;

} __attribute__((packed)) queue_struct;

//basic queue functions and tree functions
queue_node* init_queue_node(Quadtree* node)
{

    queue_node* new_node = malloc(sizeof(queue_node));
    if (new_node == NULL)
        return NULL;

    new_node->data = node;
    new_node->prev = new_node->next = NULL;

    return new_node;
}

queue_struct* init_queue()
{

    queue_struct* new_queue = malloc(sizeof(queue_struct));

    if (new_queue == NULL)
        return NULL;

    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->len = 0;

    return new_queue;
}

void enqueue(queue_struct* queue, Quadtree* data)
{
    if (data == NULL) {
        return;
    }

    queue_node* new = init_queue_node(data);

    if (queue->len == 0)
    {
        queue->head = new;
        queue->tail = new;
    }
    else
    {
        new->next = queue->head;
        queue->head->prev = new;
        queue->head = new;
    }
    (queue->len)++;
}

Quadtree* dequeue(queue_struct* queue)
{
    Quadtree* ret;

    if (queue->len == 0)
        return NULL;

    queue_node* aux = queue->tail;

    if (queue->len == 1)
    {
        queue->tail = NULL;
        queue->head = NULL;
    }
    else
    {
        queue->tail = aux->prev;
        queue->tail->next = NULL;
    }
    (queue->len)--;

    ret = aux->data;

    free(aux);

    return ret;
}

Quadtree* init_node()
{
    Quadtree* node = malloc(sizeof(Quadtree));

    if (node == NULL)
        return NULL;

    node->parent = NULL;
    node->bottom_left = NULL;
    node->bottom_right = NULL;
    node->top_left = NULL;
    node->top_right = NULL;

    return node;
}

void initialize_areas(QuadtreeNode* node, int area)
{
    node->area = area;
}

//attribute colors to a block as the mean of the pixels inside it
void initialize_colors(QuadtreeNode* node, pixels_struct** pixels, int x, int y, int len_of_block)
{
    int i, j;
    long long sum_red = 0, sum_green = 0, sum_blue = 0;

    for (i = x; i < x + len_of_block; i++)
        for (j = y; j < y + len_of_block; j++)
        {
            sum_red += pixels[i][j].red;
            sum_green += pixels[i][j].green;
            sum_blue += pixels[i][j].blue;
        }

    int number_elements = len_of_block * len_of_block;

    node->red = (unsigned char)(sum_red / number_elements);
    node->green = (unsigned char)(sum_green / number_elements);
    node->blue = (unsigned char)(sum_blue / number_elements);
}

//create a quad tree from pixels matrix
void create_tree_from_pixels(Quadtree* tree, pixels_struct** pixels, int x, int y, int area, int len_of_block)
{
    if (area > 1)
    {
        Quadtree* top_left = init_node();
        Quadtree* top_right = init_node();
        Quadtree* bot_right = init_node();
        Quadtree* bot_left = init_node();

        //links with parent
        top_left->parent = tree;
        top_right->parent = tree;
        bot_right->parent = tree;
        bot_left->parent = tree;

        //links with children
        tree->top_left = top_left;
        tree->top_right = top_right;
        tree->bottom_right = bot_right;
        tree->bottom_left = bot_left;

        //datas
        initialize_areas(&tree->data, area);
        initialize_colors(&tree->data, pixels, x, y, len_of_block);

        //continue creating tree nodes
        create_tree_from_pixels(top_left, pixels, x, y, area / 4, len_of_block / 2);
        create_tree_from_pixels(top_right, pixels, x, y + len_of_block / 2, area / 4, len_of_block / 2);
        create_tree_from_pixels(bot_right, pixels, x + len_of_block / 2, y + len_of_block / 2, area / 4, len_of_block / 2);
        create_tree_from_pixels(bot_left, pixels, x + len_of_block / 2, y, area / 4, len_of_block / 2);
    }
    else
    if (area == 1)
    {

        tree->data.red = pixels[x][y].red;
        tree->data.green = pixels[x][y].green;
        tree->data.blue = pixels[x][y].blue;

        tree->data.area = 1;

        tree->data.top_left = -1;
        tree->data.top_right = -1;
        tree->data.bottom_left = -1;
        tree->data.bottom_right = -1;
    }
    else return;

}


void destroy_tree(Quadtree* tree)
{
    if (tree == NULL) {
        return;
    }

    destroy_tree(tree->top_left);
    destroy_tree(tree->top_right);
    destroy_tree(tree->bottom_right);
    destroy_tree(tree->bottom_left);

    free(tree);
    tree = NULL;

}

void make_leaf(Quadtree* tree)
{
    //rewrite data in struct
    tree->data.top_left = -1;
    tree->data.top_right = -1;
    tree->data.bottom_left = -1;
    tree->data.bottom_right = -1;

    //delete
    destroy_tree(tree->top_left);
    destroy_tree(tree->top_right);
    destroy_tree(tree->bottom_left);
    destroy_tree(tree->bottom_right);

    //for additional deletes
    tree->top_left = NULL;
    tree->top_right = NULL;
    tree->bottom_right = NULL;
    tree->bottom_left = NULL;

}

//compress tree using the specified format and formula
void compress_tree(Quadtree* tree, pixels_struct** pixels, int x, int y, int len_of_block, int format)
{
    int i, j;
    uint64_t sum = 0;
    if (len_of_block > 1)
    {

        for (i = x; i < x + len_of_block; i++)
            for (j = y; j < y + len_of_block; j++)
                sum += ((uint64_t)(tree->data.red - pixels[i][j].red)) * ((uint64_t)(tree->data.red - pixels[i][j].red)) + ((uint64_t)(tree->data.green - pixels[i][j].green)) * ((uint64_t)(tree->data.green - pixels[i][j].green)) + ((uint64_t)(tree->data.blue - pixels[i][j].blue)) * ((uint64_t)(tree->data.blue - pixels[i][j].blue));

        uint64_t mean = (sum / ((uint64_t)len_of_block * (uint64_t)len_of_block * 3));
        if (mean <= format)
            make_leaf(tree);
        else
        {
            compress_tree(tree->top_left, pixels, x, y, len_of_block / 2, format);
            compress_tree(tree->top_right, pixels, x, y + len_of_block / 2, len_of_block / 2, format);
            compress_tree(tree->bottom_right, pixels, x + len_of_block / 2, y + len_of_block / 2, len_of_block / 2, format);
            compress_tree(tree->bottom_left, pixels, x + len_of_block / 2, y, len_of_block / 2, format);
        }
    }
    else
        return;
}

int get_number_nodes(Quadtree* tree)
{
    if (tree == NULL)
        return 0;
    else
    {
        return 1 + get_number_nodes(tree->top_left) + get_number_nodes(tree->top_right) + get_number_nodes(tree->bottom_right) + get_number_nodes(tree->bottom_left);
    }
}

//test if node is leaf by testing tree linkings
int is_leaf(Quadtree* node)
{
    return (node->top_left == NULL) && (node->top_right == NULL) && (node->bottom_left == NULL) && (node->bottom_right == NULL);
}

int get_number_leaves(Quadtree* tree)
{
    if (is_leaf(tree))
        return 1;
    else
        return get_number_leaves(tree->top_left) + get_number_leaves(tree->top_right) + get_number_leaves(tree->bottom_left) + get_number_leaves(tree->bottom_right);
}

//number the tree nodes
void put_index_in_tree(Quadtree* tree)
{
    int index = 0;
    if (tree == NULL)
        return;

    queue_struct* bfs_queue = init_queue();

    enqueue(bfs_queue, tree);

    while (bfs_queue->len != 0)
    {
        Quadtree* queue_dequeued = dequeue(bfs_queue);

        queue_dequeued->index = index;
        index++;

        if (!is_leaf(queue_dequeued))
        {
            enqueue(bfs_queue, queue_dequeued->top_left);
            enqueue(bfs_queue, queue_dequeued->top_right);
            enqueue(bfs_queue, queue_dequeued->bottom_right);
            enqueue(bfs_queue, queue_dequeued->bottom_left);
        }
    }
    free(bfs_queue);
}

//number the QuadtreeNode data part of the Quadtree
void put_children_indexes(Quadtree* tree)
{
    if (!is_leaf(tree))
    {
        tree->data.top_left = tree->top_left->index;
        tree->data.top_right = tree->top_right->index;
        tree->data.bottom_left = tree->bottom_left->index;
        tree->data.bottom_right = tree->bottom_right->index;

        put_children_indexes(tree->top_left);
        put_children_indexes(tree->top_right);
        put_children_indexes(tree->bottom_right);
        put_children_indexes(tree->bottom_left);
    }
    else
        return;
}

//put data from tree in an array(v)
void insert_nodes(Quadtree* tree, QuadtreeNode* v, int* index)
{
    if (tree == NULL)
        return;

    queue_struct* bfs_queue = init_queue();

    enqueue(bfs_queue, tree);

    while (bfs_queue->len != 0)
    {
        Quadtree* queue_dequeued = dequeue(bfs_queue);

        memcpy((v + *index), &queue_dequeued->data, sizeof(QuadtreeNode));
        (*index)++;

        if (!is_leaf(queue_dequeued))
        {
            enqueue(bfs_queue, queue_dequeued->top_left);
            enqueue(bfs_queue, queue_dequeued->top_right);
            enqueue(bfs_queue, queue_dequeued->bottom_right);
            enqueue(bfs_queue, queue_dequeued->bottom_left);
        }
    }
    free(bfs_queue);
}

//check if leaf by checking the QuadtreeNode data part of Quadtree
int is_leaf_from_array(QuadtreeNode* data)
{
    return ((data->top_left == -1) && (data->top_right == -1) && (data->bottom_right == -1) && (data->bottom_left == -1));
}

//initialise a tree knowing the compressed array
void init_tree_from_array(Quadtree* tree, QuadtreeNode* v, int* index, int number_nodes)
{
    memmove(&tree->data, v + *index, sizeof(QuadtreeNode));
    if (*index == number_nodes)
        return;

    if (!is_leaf_from_array(&v[*index]))
    {

        //init children
        Quadtree* top_left = init_node();
        Quadtree* top_right = init_node();
        Quadtree* bot_right = init_node();
        Quadtree* bot_left = init_node();

        //links with parent
        top_left->parent = tree;
        top_right->parent = tree;
        bot_right->parent = tree;
        bot_left->parent = tree;

        //links with children
        tree->top_left = top_left;
        tree->top_right = top_right;
        tree->bottom_right = bot_right;
        tree->bottom_left = bot_left;

        //finished current element, go to next(in the array)
        (*index)++;

        init_tree_from_array(top_left, v, index, number_nodes);
        init_tree_from_array(top_right, v, index, number_nodes);
        init_tree_from_array(bot_right, v, index, number_nodes);
        init_tree_from_array(bot_left, v, index, number_nodes);

    }
    else
        (*index)++;
}

void init_children_areas_and_colors(Quadtree* tree)
{
    //areas
    tree->top_left->data.area = tree->data.area / 4;
    tree->top_right->data.area = tree->data.area / 4;
    tree->bottom_right->data.area = tree->data.area / 4;
    tree->bottom_left->data.area = tree->data.area / 4;

    //colors for top_left
    tree->top_left->data.blue = tree->data.blue;
    tree->top_left->data.green = tree->data.green;
    tree->top_left->data.red = tree->data.red;

    //colors for top_right
    tree->top_right->data.blue = tree->data.blue;
    tree->top_right->data.green = tree->data.green;
    tree->top_right->data.red = tree->data.red;

    //colors for bottom_right
    tree->bottom_right->data.blue = tree->data.blue;
    tree->bottom_right->data.green = tree->data.green;
    tree->bottom_right->data.red = tree->data.red;

    //colors for bottom_left
    tree->bottom_left->data.blue = tree->data.blue;
    tree->bottom_left->data.green = tree->data.green;
    tree->bottom_left->data.red = tree->data.red;

}

//if a leaf does not have the area = 1, then it is not a pixel, so we break the block:
void partition_area_into_pixels(Quadtree* tree)
{
    if (is_leaf(tree) && tree->data.area != 1)
    {
        //init children
        Quadtree* top_left = init_node();
        Quadtree* top_right = init_node();
        Quadtree* bot_right = init_node();
        Quadtree* bot_left = init_node();

        //links with parent
        top_left->parent = tree;
        top_right->parent = tree;
        bot_right->parent = tree;
        bot_left->parent = tree;

        //links with children
        tree->top_left = top_left;
        tree->top_right = top_right;
        tree->bottom_right = bot_right;
        tree->bottom_left = bot_left;

        init_children_areas_and_colors(tree);

        partition_area_into_pixels(tree->top_left);
        partition_area_into_pixels(tree->top_right);
        partition_area_into_pixels(tree->bottom_right);
        partition_area_into_pixels(tree->bottom_left);
    }
    else
    if (!is_leaf(tree))
    {
        partition_area_into_pixels(tree->top_left);
        partition_area_into_pixels(tree->top_right);
        partition_area_into_pixels(tree->bottom_right);
        partition_area_into_pixels(tree->bottom_left);
    }
    else
    if (tree->data.area == 1)
        return;
}

//on last level we have the colors
queue_struct* enqueue_last_level(Quadtree* tree)
{
    queue_struct* last_level = init_queue();
    queue_struct* bfs_queue = init_queue();

    enqueue(bfs_queue, tree);

    while (bfs_queue->len != 0)
    {
        Quadtree* queue_dequeued = dequeue(bfs_queue);

        if (is_leaf(queue_dequeued))
            enqueue(last_level, queue_dequeued);

        if (!is_leaf(queue_dequeued))
        {
            enqueue(bfs_queue, queue_dequeued->top_left);
            enqueue(bfs_queue, queue_dequeued->top_right);
            enqueue(bfs_queue, queue_dequeued->bottom_right);
            enqueue(bfs_queue, queue_dequeued->bottom_left);
        }
    }
    free(bfs_queue);

    return last_level;
}

//from array to matrix of pixels
void put_pixels_in_matrix(pixels_struct** pixels, QuadtreeNode* arr, int x, int y, int len_of_block, int* iteration)
{

    if (len_of_block == 1)
    {

        pixels[x][y].red = arr[(*iteration)].red;
        pixels[x][y].green = arr[(*iteration)].green;
        pixels[x][y].blue = arr[(*iteration)].blue;

        (*iteration) += 1;
    }
    else
    {
        put_pixels_in_matrix(pixels, arr, x, y, len_of_block / 2, iteration);
        put_pixels_in_matrix(pixels, arr, x, y + len_of_block / 2, len_of_block / 2, iteration);
        put_pixels_in_matrix(pixels, arr, x + len_of_block / 2, y + len_of_block / 2, len_of_block / 2, iteration);
        put_pixels_in_matrix(pixels, arr, x + len_of_block / 2, y, len_of_block / 2, iteration);


    }

}

//swap all node data (parents and children included)
void swap_nodes(Quadtree* parent1, Quadtree* parent2)
{
    Quadtree* top_left1, * top_left2, * top_right1, * top_right2;
    Quadtree* bottom_left1, * bottom_left2, * bottom_right1, * bottom_right2;

    //memorize children
    top_left1 = parent1->top_left;
    top_right1 = parent1->top_right;
    bottom_left1 = parent1->bottom_left;
    bottom_right1 = parent1->bottom_right;

    top_left2 = parent2->top_left;
    top_right2 = parent2->top_right;
    bottom_left2 = parent2->bottom_left;
    bottom_right2 = parent2->bottom_right;


    //redo children
    parent1->top_left = top_left2;
    parent1->top_right = top_right2;
    parent1->bottom_left = bottom_left2;
    parent1->bottom_right = bottom_right2;

    parent2->top_left = top_left1;
    parent2->top_right = top_right1;
    parent2->bottom_left = bottom_left1;
    parent2->bottom_right = bottom_right1;


    //redo parents
    top_left1->parent = parent2;
    top_right1->parent = parent2;
    bottom_left1->parent = parent2;
    bottom_right1->parent = parent2;

    top_left2->parent = parent1;
    top_right2->parent = parent1;
    bottom_left2->parent = parent1;
    bottom_right2->parent = parent1;
}

//almost the same as previous function
queue_struct* enqueue_almost_last_level(Quadtree* tree)
{
    queue_struct* last_level = init_queue();
    queue_struct* bfs_queue = init_queue();

    enqueue(bfs_queue, tree);

    while (bfs_queue->len != 0)
    {
        Quadtree* queue_dequeued = dequeue(bfs_queue);

        if (is_leaf(queue_dequeued->top_left))
            enqueue(last_level, queue_dequeued);

        if (!is_leaf(queue_dequeued->top_left))
        {
            enqueue(bfs_queue, queue_dequeued->top_left);
            enqueue(bfs_queue, queue_dequeued->top_right);
            enqueue(bfs_queue, queue_dequeued->bottom_right);
            enqueue(bfs_queue, queue_dequeued->bottom_left);
        }
    }
    free(bfs_queue);

    return last_level;
}

//mirror the array by reading the nodes in an opposite order
void mirror_vertical_1(Quadtree* tree)
{
    int i;
    queue_struct* almost_last_level = enqueue_almost_last_level(tree);

    Quadtree** arr = malloc(almost_last_level->len * sizeof(Quadtree));
    if (arr == NULL)
        return;
    int lenght = almost_last_level->len;

    for (i = 0; i < lenght; i++)
        arr[i] = dequeue(almost_last_level);

    for (i = 0; i < lenght / 2; i++)
        swap_nodes(arr[i], arr[lenght - i - 1]);

}

//read pixels from .ppm file
pixels_struct** read_pixels(char* input_file, int* width, int* height)
{
    char p6[3], whitespace, digit;
    int i, j, max_color;


    FILE* fr = fopen(input_file, "rb+");

    if (fr == NULL)
    {
        printf("error at opening file\n");
        return NULL;
    }

    fread(&p6, sizeof(char), 2, fr);
    fread(&whitespace, sizeof(char), 1, fr);

    fread(&digit, sizeof(char), 1, fr);
    while (digit != ' ' && digit != '\n')
    {
        *width = (*width) * 10 + (digit - '0');
        fread(&digit, sizeof(char), 1, fr);
    }

    fread(&digit, sizeof(char), 1, fr);
    while (digit != ' ' && digit != '\n')
    {
        *height = (*height) * 10 + (digit - '0');
        fread(&digit, sizeof(char), 1, fr);
    }

    max_color = 0;
    fread(&digit, sizeof(char), 1, fr);
    while (digit != ' ' && digit != '\n')
    {
        max_color = max_color * 10 + (digit - '0');
        fread(&digit, sizeof(char), 1, fr);
    }

    pixels_struct** pixels = (pixels_struct**)malloc((*height) * sizeof(pixels_struct*));
    if (pixels == NULL)
        return NULL;
    for (i = 0; i < *height; i++)
    {
        pixels[i] = (pixels_struct*)malloc(sizeof(pixels_struct) * (*width));
        if (pixels[i] == NULL)
            return NULL;
    }

    for (i = 0; i < *height; i++)
        for (j = 0; j < *width; j++)
        {
            fread(&pixels[i][j].red, sizeof(char), 1, fr);
            fread(&pixels[i][j].green, sizeof(char), 1, fr);
            fread(&pixels[i][j].blue, sizeof(char), 1, fr);
        }


    fclose(fr);
    return pixels;
}

//write compressed file
void write_file_task_1(QuadtreeNode* v, char* output_file, int colors, int nodes)
{
    int i;

    FILE* fw = fopen(output_file, "wb+");
    if (fw == NULL)
        return;
    fwrite(&colors, sizeof(int), 1, fw);
    fwrite(&nodes, sizeof(int), 1, fw);
    for (i = 0; i < nodes; i++)
    {
        fwrite(&(v[i].blue), 1, 1, fw);
        fwrite(&(v[i].green), 1, 1, fw);
        fwrite(&(v[i].red), 1, 1, fw);

        fwrite(&v[i].area, 4, 1, fw);

        fwrite(&v[i].top_left, 4, 1, fw);
        fwrite(&v[i].top_right, 4, 1, fw);
        fwrite(&v[i].bottom_left, 4, 1, fw);
        fwrite(&v[i].bottom_right, 4, 1, fw);
    }
    fclose(fw);
}

//read compressed array
QuadtreeNode* read_file_task2(char* input_file, int* color_number, int* nodes_number)
{
    FILE* fr = fopen(input_file, "rb");

    if (fr == NULL)
    {
        printf("error at opening file");
        return NULL;
    }

    fread(color_number, sizeof(unsigned int), 1, fr);
    fread(nodes_number, sizeof(unsigned int), 1, fr);

    QuadtreeNode* v = malloc((*nodes_number) * sizeof(QuadtreeNode));
    if (v == NULL)
        return NULL;

    fread(v, sizeof(QuadtreeNode), *nodes_number, fr);
    fclose(fr);

    return v;
}

//transform a tree into a pixels matrix and write it to the file
void write_tree_to_ppm(char* output_file, Quadtree* tree, int width, int height, pixels_struct** pixels)
{
    int i, j;
    FILE* fw = fopen(output_file, "wb+");
    if (fw == NULL)
        return;
    fwrite("P6\n", 3, 1, fw);

    int nr_digits = 0;
    char* new_width = malloc(nr_digits * sizeof(char));
    int copy = width;
    while (copy != 0)
    {
        copy = copy / 10;
        nr_digits++;
    }
    copy = width;
    int last_digit;
    for (i = 0; i < nr_digits; i++)
    {
        last_digit = copy % 10;
        copy = copy / 10;
        new_width[nr_digits - i - 1] = last_digit + '0';
    }
    fwrite(new_width, nr_digits * sizeof(char), 1, fw);

    fwrite(" ", 1, 1, fw);
    fwrite(new_width, nr_digits * sizeof(char), 1, fw);
    fwrite("\n", 1, 1, fw);
    fwrite("255\n", 4, 1, fw);

    //header of the file


    queue_struct* last_level = enqueue_last_level(tree);
    QuadtreeNode* arr = malloc(last_level->len * sizeof(QuadtreeNode));

    int lenght = last_level->len;

    for (i = 0; i < lenght; i++)
    {
        Quadtree* node = dequeue(last_level);
        memcpy(&arr[i], &node->data, sizeof(QuadtreeNode));
    }

    int iteration = 0;
    put_pixels_in_matrix(pixels, arr, 0, 0, width, &iteration);

    for (i = 0; i < width; i++)
        for (j = 0; j < width; j++)
        {
            fwrite(&pixels[i][j].red, 1, 1, fw);
            fwrite(&pixels[i][j].green, 1, 1, fw);
            fwrite(&pixels[i][j].blue, 1, 1, fw);
        }

    fclose(fw);
    free(arr);
    free(last_level);
    free(new_width);
}
int main(int argc, char* argv[])
{
    int width = 0, height = 0, i, nodes, colors;

    char input_file[20], output_file[20], factor[4], command[2], type[2];

    if (argc == 5)
    {
        strcpy(input_file, argv[3]);
        strcpy(output_file, argv[4]);
        strcpy(command, argv[1]);
    }
    else
    if (argc == 4)
    {
        strcpy(input_file, argv[2]);
        strcpy(output_file, argv[3]);
        strcpy(command, argv[1]);
    }
    else
    {
        strcpy(command, argv[1]);
        strcpy(type, argv[2]);
        strcpy(factor, argv[3]);
        strcpy(input_file, argv[4]);
        strcpy(output_file, argv[5]);
    }

    //task1
    if (strcmp(command, "-c") == 0)
    {
        width = 0;
        height = 0;
        pixels_struct** pixels = read_pixels(input_file, &width, &height);

        Quadtree* tree = init_node();
        create_tree_from_pixels(tree, pixels, 0, 0, width * height, width);

        strcpy(factor, argv[2]);
        int factor_int = atoi(factor);

        //compress tree and number the nodes
        compress_tree(tree, pixels, 0, 0, width, factor_int);
        put_index_in_tree(tree);
        put_children_indexes(tree);

        //create array
        colors = get_number_leaves(tree);
        nodes = get_number_nodes(tree);
        QuadtreeNode* v = malloc(nodes * sizeof(QuadtreeNode));

        if (v == NULL)
            return -1;
        int index = 0;

        insert_nodes(tree, v, &index);

        write_file_task_1(v, output_file, colors, nodes);

        free(v);
        destroy_tree(tree);
        for (i = 0; i < height; i++)
            free(pixels[i]);
        free(pixels);
        return 0;
    }

    if (strcmp(command, "-d") == 0) //decompress case
    {
        int color_number = 0, nodes_number = 0;

        QuadtreeNode* v = read_file_task2(input_file, &color_number, &nodes_number);

        uint64_t full_area = v[0].area;
        int width = (int)sqrt(full_area), height;
        height = width;

        pixels_struct** pixels = (pixels_struct**)malloc(height * sizeof(pixels_struct*));
        if (pixels == NULL)
            return -7;
        for (i = 0; i < height; i++)
        {
            pixels[i] = (pixels_struct*)malloc(sizeof(pixels_struct) * width);
            if (pixels[i] == NULL)
                return -7;
        }

        int index = 0;
        //create tree
        Quadtree* tree = init_node();
        init_tree_from_array(tree, v, &index, nodes_number);
        partition_area_into_pixels(tree);

        //write result to the output image
        write_tree_to_ppm(output_file, tree, width, height, pixels);

        destroy_tree(tree);
        free(v);
        for (i = 0; i < height; i++)
            free(pixels[i]);
        free(pixels);
    }

    if (strncmp(command, "-m", 2) == 0)
    {
        width = 0;
        height = 0;
        pixels_struct** pixels = read_pixels(input_file, &width, &height);

        //create tree from .ppm
        Quadtree* tree = init_node();
        create_tree_from_pixels(tree, pixels, 0, 0, width * height, width);
        strcpy(factor, argv[3]);
        int factor_int = atoi(factor);
        compress_tree(tree, pixels, 0, 0, width, factor_int);
        partition_area_into_pixels(tree);
        put_index_in_tree(tree);
        put_children_indexes(tree);

        //mirror it
        mirror_vertical_1(tree);

        //write the output tree as we did at task2
        queue_struct* last_level = enqueue_last_level(tree);
        QuadtreeNode* arr = malloc(last_level->len * sizeof(QuadtreeNode));

        int lenght = last_level->len;

        for (i = 0; i < lenght; i++)
        {
            Quadtree* node = dequeue(last_level);
            memcpy(&arr[i], &node->data, sizeof(QuadtreeNode));
        }

        int iteration = 0;
        put_pixels_in_matrix(pixels, arr, 0, 0, width, &iteration);
        write_tree_to_ppm(output_file, tree, width, height, pixels);
        destroy_tree(tree);
        for (i = 0; i < height; i++)
            free(pixels[i]);
        free(pixels);
        return 0;
    }

    return 0;
}