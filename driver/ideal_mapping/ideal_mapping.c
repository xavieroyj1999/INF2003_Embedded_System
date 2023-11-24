/* DISCLAIMER
ChatGPT is used in this file. Stack and Queue baseline was created with geeks4geeks stack/queue example fed into ChatGPT and modified from then on.

Thus Queue and Stack functions are not original, but the rest of the code is original.*/

#include <stdio.h>
#include <stdlib.h>

#define MIN_ARRAY_SIZE 0
#define MAX_ARRAY_SIZE 8
#define MAX_SIZE 9

typedef enum { false, true } bool;

enum directionEvent {
    NORTH = 0x01,
    EAST = 0x02,
    SOUTH = 0x04,
    WEST = 0x08
};

enum directionDegree {
    UP = 0,
    RIGHT = 90,
    DOWN = 180,
    LEFT = 270
};

struct cell {
    int walls;
    int cost;
    int start;
    int goal;
    bool visited;
    bool barcode;
};

void mapInit(struct cell map[MAX_SIZE][MAX_SIZE]) {
    for (int j = MIN_ARRAY_SIZE; j <= MAX_ARRAY_SIZE; j++) {
        for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
            map[i][j].walls = 0x00;
            map[i][j].visited = false;
            map[i][j].barcode = false;
            map[i][j].cost = 0;
            map[i][j].start = false;
            map[i][j].goal = false;

            if (i == MIN_ARRAY_SIZE) map[i][j].walls |= WEST;
            if (i == MAX_ARRAY_SIZE) map[i][j].walls |= EAST;
            if (j == MIN_ARRAY_SIZE) map[i][j].walls |= NORTH;
            if (j == MAX_ARRAY_SIZE) map[i][j].walls |= SOUTH;
        }
    }
}

void resetVisited(struct cell map[MAX_SIZE][MAX_SIZE]) {
    for (int j = MIN_ARRAY_SIZE; j <= MAX_ARRAY_SIZE; j++) {
        for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
            map[i][j].visited = false;
        }
    }
}

void setWall(struct cell map[MAX_SIZE][MAX_SIZE], int x, int y, enum directionDegree carDirection) {
    if (carDirection == UP) {
        map[x][y].walls |= NORTH;
        map[x][y - 1].walls |= SOUTH;
    }
    else if (carDirection == RIGHT) {
        map[x][y].walls |= EAST;
        map[x + 1][y].walls |= WEST;
    }
    else if (carDirection == DOWN) {
        map[x][y].walls |= SOUTH;
        map[x][y + 1].walls |= NORTH;
    }
    else if (carDirection == LEFT) {
        map[x][y].walls |= WEST;
        map[x - 1][y].walls |= EAST;
    }
}

void mapBarcode(struct cell map[MAX_SIZE][MAX_SIZE], int x, int y) {
    map[x][y].barcode = true;
}

void cellVisited(struct cell map[MAX_SIZE][MAX_SIZE], int x, int y) {
    map[x][y].visited = true;
}

void setStart(struct cell map[MAX_SIZE][MAX_SIZE], int x, int y) {
    map[x][y].start = true;
    map[x][y].walls &= ~SOUTH;
}

void setGoal(struct cell map[MAX_SIZE][MAX_SIZE], int x, int y) {
    map[x][y].goal = true;
    map[x][y].walls &= ~NORTH;
}

void printMap(struct cell map[MAX_SIZE][MAX_SIZE]) {
    for (int j = MIN_ARRAY_SIZE; j <= MAX_ARRAY_SIZE; j++) {
        for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
            if (map[i][j].walls & NORTH) {
                printf("----");
            } else {
                printf("    ");
            }
        }
        printf("\n");
        for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
            printf("%c%d%d%c", (map[i][j].walls & WEST) ? '|' : ' ', i, j, (map[i][j].walls & EAST) ? '|' : ' ');
        }
        printf("\n");
        if (j == MAX_ARRAY_SIZE) {
            for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
                if (map[i][j].walls & SOUTH) {
                    printf("----");
                } else {
                    printf("    ");
                }
            }
            printf("\n");
        }
    }
}

void printVisitedMap(struct cell map[MAX_SIZE][MAX_SIZE]) {
    for (int j = MIN_ARRAY_SIZE; j <= MAX_ARRAY_SIZE; j++) {
        for (int i = MIN_ARRAY_SIZE; i <= MAX_ARRAY_SIZE; i++) {
            if (map[i][j].visited == true) {
                printf(" 1  ");
            }
            else {
                printf(" 0  ");
            }
        }
        printf("\n");
    }
}

struct coordinates {
    int x_coord;
    int y_coord;
};

struct car {
    int initialDegree;
    int carDirection;
    struct coordinates current_location;
};

void initCar(struct car* myCar) {
    myCar->initialDegree = 0;
    myCar->carDirection = 0;
    myCar->current_location.x_coord = 0;
    myCar->current_location.y_coord = 0;
}

void orientateCar(struct car* myCar, int degree) {
    myCar->carDirection = (myCar->initialDegree - degree) % 360;
}

void moveCar(struct car* myCar, int x, int y) {
    myCar->current_location.x_coord += x;
    myCar->current_location.y_coord += y;
}

void setCarStart(struct car* myCar, int x, int y) {
    myCar->current_location.x_coord = x;
    myCar->current_location.y_coord = y;
}

struct StackNode {
    struct coordinates* data;
    struct StackNode* next;
};

struct Stack {
    struct StackNode* top;
};

struct Stack* createStack() {
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->top = NULL;
    return stack;
}

void push(struct Stack* stack, struct coordinates data) {
    struct StackNode* newNode = (struct StackNode*)malloc(sizeof(struct StackNode));
    newNode->data = (struct coordinates*)malloc(sizeof(struct coordinates));
    newNode->data->x_coord = data.x_coord;
    newNode->data->y_coord = data.y_coord;
    newNode->next = stack->top;
    stack->top = newNode;
}

void pop(struct Stack* stack) {
    struct StackNode* temp = stack->top;
    stack->top = stack->top->next;
    free(temp);
}

bool isStackEmpty(struct Stack* stack) {
    if (stack->top == NULL) {
        return true;
    }
    else {
        return false;
    }
}

bool explore_map(struct cell map[MAX_SIZE][MAX_SIZE], struct Stack* stack) {
    if (isStackEmpty(stack)) {
        return false;
    }

    int current_x = stack->top->data->x_coord;
    int current_y = stack->top->data->y_coord;

    if (!(map[current_x][current_y].walls & NORTH) && (current_y > MIN_ARRAY_SIZE) && map[current_x][current_y - 1].visited == false) {
        struct coordinates north_coord = { current_x, current_y - 1 };
        cellVisited(map, current_x, current_y - 1);
        push(stack, north_coord);
    }
    else if (!(map[current_x][current_y].walls & EAST) && (current_x < MAX_ARRAY_SIZE) && map[current_x + 1][current_y].visited == false) {
        struct coordinates east_coord = { current_x + 1, current_y };
        cellVisited(map, current_x + 1, current_y);
        push(stack, east_coord);
    }
    else if (!(map[current_x][current_y].walls & SOUTH) && (current_y < MAX_ARRAY_SIZE) && map[current_x][current_y + 1].visited == false) {
        struct coordinates south_coord = { current_x, current_y + 1 };
        cellVisited(map, current_x, current_y + 1);
        push(stack, south_coord);
    }
    else if (!(map[current_x][current_y].walls & WEST) && (current_x > MIN_ARRAY_SIZE) && map[current_x - 1][current_y].visited == false) {
        struct coordinates west_coord = { current_x - 1, current_y };
        cellVisited(map, current_x - 1, current_y);
        push(stack, west_coord);
    }
    else {
        pop(stack);
    }
    return true;
}

struct QNode {
    struct coordinates* key;
    struct QNode* prev;
    struct QNode* next;
};

struct Queue {
    struct QNode* head, * tail;
};

struct QNode* newNode(struct coordinates my_coordinates) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = (struct coordinates*)malloc(sizeof(struct coordinates));
    temp->key->x_coord = my_coordinates.x_coord;
    temp->key->y_coord = my_coordinates.y_coord;
    temp->prev = NULL;
    temp->next = NULL;
    return temp;
}

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->head = q->tail = NULL;
    return q;
}

void enQueue(struct Queue* q, struct coordinates my_coordinates) {
    struct QNode* temp = newNode(my_coordinates);
    if (q->tail == NULL) {
        q->head = q->tail = temp;
        return;
    }
    q->tail->next = temp;
    temp->prev = q->tail;
    q->tail = temp;
}

void deQueue(struct Queue* q) {
    if (q->head == NULL)
        return;
    struct QNode* temp = q->head;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    if (temp->next != NULL) {
        temp->next->prev = NULL;
    }
    free(temp->key);
    free(temp);
}

void clearQueueUntilOne(struct Queue* q) {
    while (q->head != NULL && q->head != q->tail) {
        struct QNode* temp = q->head;
        q->head = q->head->next;
        if (q->head != NULL) {
            q->head->prev = NULL;
        }
        free(temp->key);
        free(temp);
    }
}

void printQueue(struct Queue* q) {
    struct QNode* temp = q->head;
    while (temp != NULL) {
        printf("(%d, %d) ", temp->key->x_coord, temp->key->y_coord);
        temp = temp->next;
    }
    printf("\n");
}

void printQueueBackward(struct Queue* q) {
    struct QNode* temp = q->tail;
    while (temp != NULL) {
        printf("(%d, %d) ", temp->key->x_coord, temp->key->y_coord);
        temp = temp->prev;
    }
    printf("\n");
}


bool assign_cost_cell(struct cell map[MAX_SIZE][MAX_SIZE], struct Queue* q) {
    int current_x = q->head->key->x_coord;
    int current_y = q->head->key->y_coord;

    int current_cost = map[current_x][current_y].cost;
    cellVisited(map, current_x, current_y);
    if (!(map[current_x][current_y].walls & NORTH) && (current_y > MIN_ARRAY_SIZE) && map[current_x][current_y - 1].visited == false) {
        map[current_x][current_y - 1].cost = current_cost + 1;
        struct coordinates north_coord = { current_x, current_y - 1 };
        cellVisited(map, current_x, current_y - 1);
        enQueue(q, north_coord);
        if (map[current_x][current_y - 1].goal == true) return false;
    }
    if (!(map[current_x][current_y].walls & EAST) && (current_x < MAX_ARRAY_SIZE) && map[current_x + 1][current_y].visited == false) {
        map[current_x + 1][current_y].cost = current_cost + 1;
        struct coordinates east_coord = { current_x + 1, current_y };
        cellVisited(map, current_x + 1, current_y);
        enQueue(q, east_coord);
        if (map[current_x + 1][current_y].goal == true) return false;
    }
    if (!(map[current_x][current_y].walls & SOUTH) && (current_y < MAX_ARRAY_SIZE) && map[current_x][current_y + 1].visited == false) {
        map[current_x][current_y + 1].cost = current_cost + 1;
        struct coordinates south_coord = { current_x, current_y + 1 };
        cellVisited(map, current_x, current_y + 1);
        enQueue(q, south_coord);
        if (map[current_x][current_y + 1].goal == true) return false;
    }
    if (!(map[current_x][current_y].walls & WEST) && (current_x > MIN_ARRAY_SIZE) && map[current_x - 1][current_y].visited == false) {
        map[current_x - 1][current_y].cost = current_cost + 1;
        struct coordinates west_coord = { current_x - 1, current_y };
        cellVisited(map, current_x - 1, current_y);
        enQueue(q, west_coord);
        if (map[current_x - 1][current_y].goal == true) return false;
    }
    return true;
}

bool get_shortest_path(struct cell map[MAX_SIZE][MAX_SIZE], struct Queue* q) {
    int current_x = q->tail->key->x_coord;
    int current_y = q->tail->key->y_coord;

    if (!(map[current_x][current_y].walls & NORTH) && (current_y > MIN_ARRAY_SIZE) && (map[current_x][current_y - 1].cost < map[current_x][current_y].cost) && map[current_x][current_y - 1].visited == true) {
        struct coordinates north_coord = { current_x, current_y - 1 };
        enQueue(q, north_coord);
        if (map[current_x][current_y - 1].cost == 0) return false;
    }
    else if (!(map[current_x][current_y].walls & EAST) && (current_x < MAX_ARRAY_SIZE) && (map[current_x + 1][current_y].cost < map[current_x][current_y].cost) && map[current_x + 1][current_y].visited == true) {
        struct coordinates east_coord = { current_x + 1, current_y };
        enQueue(q, east_coord);
        if (map[current_x + 1][current_y].cost == 0) return false;
    }
    else if (!(map[current_x][current_y].walls & SOUTH) && (current_y < MAX_ARRAY_SIZE) && (map[current_x][current_y + 1].cost < map[current_x][current_y].cost) && map[current_x][current_y + 1].visited == true) {
        struct coordinates south_coord = { current_x, current_y + 1 };
        enQueue(q, south_coord);
        if (map[current_x][current_y + 1].cost == 0) return false;
    }
    else if (!(map[current_x][current_y].walls & WEST) && (current_x > MIN_ARRAY_SIZE) && (map[current_x - 1][current_y].cost < map[current_x][current_y].cost) && map[current_x - 1][current_y].visited == true) {
        struct coordinates west_coord = { current_x - 1, current_y };
        enQueue(q, west_coord);
        if (map[current_x - 1][current_y].cost == 0) return false;
    }
    return true;
}

int main() {
    struct cell map[MAX_SIZE][MAX_SIZE];
    mapInit(map);

    // Manually Generate Map
    setWall(map, 0, 0, DOWN);
    setWall(map, 0, 1, RIGHT);
    setWall(map, 1, 1, DOWN);
    setWall(map, 1, 2, RIGHT);
    setWall(map, 2, 2, DOWN);
    setWall(map, 2, 3, RIGHT);
    setWall(map, 3, 3, DOWN);
    setWall(map, 3, 4, RIGHT);
    setWall(map, 4, 4, DOWN);
    setWall(map, 4, 5, RIGHT);
    setWall(map, 5, 5, DOWN);
    setWall(map, 5, 6, RIGHT);
    setWall(map, 6, 6, DOWN);

    struct Queue* q = createQueue();
    struct Stack* s = createStack();

    struct coordinates start_coord = { 0, 8 }; // Choose Start Location
    struct coordinates goal_coord = { 0, 0 }; // Choose Start Location
    enQueue(q, start_coord);
    push(s, start_coord);

    setStart(map, start_coord.x_coord, start_coord.y_coord);
    setGoal(map, goal_coord.x_coord, goal_coord.y_coord);

    while (explore_map(map, s)) {
        //printVisitedMap(map);
        //printf("\n");
    }

    printVisitedMap(map);

    resetVisited(map);

    while (assign_cost_cell(map, q)) {
        deQueue(q);
    }

    printMap(map);
    clearQueueUntilOne(q);

    while (get_shortest_path(map, q));

    printQueue(q);

    printQueueBackward(q);
}
