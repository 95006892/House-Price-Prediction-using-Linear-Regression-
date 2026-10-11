/*
 * House Price Prediction System
 * Data structures used:
 *   Array       - stores the house records
 *   Max Heap    - keeps the K nearest houses (KNN price prediction)
 *   Merge Sort  - lists houses sorted by price
 *   BST         - price range search
 *
 * Compile:  gcc main.c -o house -lm
 * Run:      ./house
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 100
#define K 3

typedef struct {
    int id, area, bhk, age;
    float price;            /* in Lakhs */
} House;

/* ---------- 1. ARRAY ---------- */
House houses[MAX];
int count = 0;
int nextId = 1;

int addHouse(int id, int area, int bhk, int age, float price) {
    if (count >= MAX) { printf("Array is full (max %d houses).\n", MAX); return 0; }
    houses[count].id = id;
    houses[count].area = area;
    houses[count].bhk = bhk;
    houses[count].age = age;
    houses[count].price = price;
    count++;
    if (id >= nextId) nextId = id + 1;
    return 1;
}

/* ---------- 2. MERGE SORT (by price) ---------- */
void merge(House a[], int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    House *L = malloc(n1 * sizeof(House));
    House *R = malloc(n2 * sizeof(House));
    for (int i = 0; i < n1; i++) L[i] = a[l + i];
    for (int j = 0; j < n2; j++) R[j] = a[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2)
        a[k++] = (L[i].price <= R[j].price) ? L[i++] : R[j++];
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
    free(L); free(R);
}

void mergeSort(House a[], int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSort(a, l, m);
    mergeSort(a, m + 1, r);
    merge(a, l, m, r);
}

void showSorted(void) {
    if (count == 0) { printf("No houses yet.\n"); return; }
    House copy[MAX];
    for (int i = 0; i < count; i++) copy[i] = houses[i];   /* keep original array untouched */
    mergeSort(copy, 0, count - 1);
    printf("\n%-4s %-10s %-5s %-6s %-10s\n", "ID", "Area", "BHK", "Age", "Price(L)");
    for (int i = 0; i < count; i++)
        printf("%-4d %-10d %-5d %-6d %-10.2f\n",
               copy[i].id, copy[i].area, copy[i].bhk, copy[i].age, copy[i].price);
}

/* ---------- 3. MAX HEAP (KNN prediction) ---------- */
/* The heap holds the K closest houses; its root is the farthest of them,
   so a new closer house simply replaces the root. */
typedef struct { House *h; double d; } HNode;
typedef struct { HNode a[K + 1]; int n; } MaxHeap;

void swapNode(HNode *x, HNode *y) { HNode t = *x; *x = *y; *y = t; }

void heapPush(MaxHeap *m, HNode x) {
    int i = m->n++;
    m->a[i] = x;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (m->a[p].d >= m->a[i].d) break;
        swapNode(&m->a[p], &m->a[i]);
        i = p;
    }
}

HNode heapPop(MaxHeap *m) {
    HNode top = m->a[0];
    m->a[0] = m->a[--m->n];
    int i = 0;
    for (;;) {
        int l = 2 * i + 1, r = l + 1, b = i;
        if (l < m->n && m->a[l].d > m->a[b].d) b = l;
        if (r < m->n && m->a[r].d > m->a[b].d) b = r;
        if (b == i) break;
        swapNode(&m->a[b], &m->a[i]);
        i = b;
    }
    return top;
}

double distance(House *h, int area, int bhk, int age) {
    double da = (area - h->area) / 100.0;
    double db = (bhk - h->bhk) * 2.0;
    double dg = (age - h->age) / 2.0;
    return sqrt(da * da + db * db + dg * dg);
}

void predictPrice(int area, int bhk, int age) {
    if (count == 0) { printf("No houses to learn from.\n"); return; }
    MaxHeap heap = { .n = 0 };
    for (int i = 0; i < count; i++) {
        HNode x = { &houses[i], distance(&houses[i], area, bhk, age) };
        if (heap.n < K) heapPush(&heap, x);
        else if (x.d < heap.a[0].d) { heapPop(&heap); heapPush(&heap, x); }
    }
    int n = heap.n;
    HNode order[K];
    for (int i = n - 1; i >= 0; i--) order[i] = heapPop(&heap);   /* nearest first */

    double sum = 0;
    for (int i = 0; i < n; i++) sum += order[i].h->price;
    printf("\nPredicted Price: %.2f Lakhs\n%d Nearest Houses:\n", sum / n, n);
    for (int i = 0; i < n; i++)
        printf("  %d. ID %d - Area: %d | BHK: %d | Age: %d | Price: %.2f L\n", i + 1,
               order[i].h->id, order[i].h->area, order[i].h->bhk,
               order[i].h->age, order[i].h->price);
}

/* ---------- 4. BINARY SEARCH TREE (price) ---------- */
typedef struct Node {
    House h;
    struct Node *left, *right;
} Node;

Node *root = NULL;

Node *insert(Node *n, House h) {
    if (!n) {
        n = malloc(sizeof(Node));
        n->h = h; n->left = n->right = NULL;
        return n;
    }
    if (h.price < n->h.price) n->left = insert(n->left, h);
    else n->right = insert(n->right, h);
    return n;
}

int rangeSearch(Node *n, float lo, float hi) {
    if (!n) return 0;
    int found = 0;
    if (lo < n->h.price) found += rangeSearch(n->left, lo, hi);
    if (n->h.price >= lo && n->h.price <= hi) {
        printf("  ID %d - Area: %d | BHK: %d | Price: %.2f L\n",
               n->h.id, n->h.area, n->h.bhk, n->h.price);
        found++;
    }
    if (hi > n->h.price) found += rangeSearch(n->right, lo, hi);
    return found;
}

void printTree(Node *n, int level) {          /* tree drawn sideways: root on the left */
    if (!n) return;
    printTree(n->right, level + 1);
    printf("%*s%.0f\n", level * 6, "", n->h.price);
    printTree(n->left, level + 1);
}

void freeTree(Node *n) {
    if (!n) return;
    freeTree(n->left); freeTree(n->right);
    free(n);
}

/* ---------- Menu ---------- */
int readInt(const char *msg, int *out) {
    printf("%s", msg);
    if (scanf("%d", out) != 1) { while (getchar() != '\n'); return 0; }
    return 1;
}
int readFloat(const char *msg, float *out) {
    printf("%s", msg);
    if (scanf("%f", out) != 1) { while (getchar() != '\n'); return 0; }
    return 1;
}

int main(void) {
    /* starting data (inserted in this order so the BST root is 70) */
    addHouse(2, 1200, 3, 3, 70); addHouse(1, 1000, 2, 5, 55);
    addHouse(3, 1500, 3, 8, 80); addHouse(6, 900, 2, 10, 40);
    addHouse(8, 1100, 2, 4, 60); addHouse(4, 1800, 4, 2, 110);
    nextId = 9;
    for (int i = 0; i < count; i++) root = insert(root, houses[i]);

    int ch;
    do {
        printf("\n===== House Price Prediction System =====\n"
               "1. Add house record        (Array)\n"
               "2. Houses sorted by price  (Merge Sort)\n"
               "3. Predict house price     (Max Heap / KNN)\n"
               "4. Price range search      (BST)\n"
               "5. Show BST\n"
               "0. Exit\n");
        if (!readInt("Choice: ", &ch)) { printf("Enter a number.\n"); ch = -1; continue; }

        if (ch == 1) {
            int a, b, g; float p;
            if (!readInt("Area (sq ft): ", &a) || !readInt("Bedrooms: ", &b) ||
                !readInt("Age (years): ", &g) || !readFloat("Price (Lakhs): ", &p) ||
                a <= 0 || b <= 0 || g < 0 || p <= 0) { printf("Invalid input.\n"); continue; }
            if (addHouse(nextId, a, b, g, p)) {
                root = insert(root, houses[count - 1]);
                printf("House ID %d added.\n", houses[count - 1].id);
            }
        } else if (ch == 2) {
            showSorted();
        } else if (ch == 3) {
            int a, b, g;
            if (!readInt("Area (sq ft): ", &a) || !readInt("Bedrooms: ", &b) ||
                !readInt("Age (years): ", &g)) { printf("Invalid input.\n"); continue; }
            predictPrice(a, b, g);
        } else if (ch == 4) {
            float lo, hi;
            if (!readFloat("Min price (Lakhs): ", &lo) || !readFloat("Max price (Lakhs): ", &hi)) {
                printf("Invalid input.\n"); continue; }
            printf("Houses in price range %.0f - %.0f:\n", lo, hi);
            if (rangeSearch(root, lo, hi) == 0) printf("  No houses found.\n");
        } else if (ch == 5) {
            printf("\nBST (root on the left):\n");
            printTree(root, 0);
        } else if (ch != 0) {
            printf("Invalid choice.\n");
        }
    } while (ch != 0);

    freeTree(root);
    return 0;
}
