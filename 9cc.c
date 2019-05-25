#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型
typedef struct {
    int ty;
    int val;
    char *input;
} Token;

// ポジション
int pos = 0;
// 入力プログラム
char *user_input;
// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンは来ないものとする
Token tokens[100];

// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_EOF
};

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;

    struct Node *lhs;
    struct Node *rhs;
    int val;
} Node;

// プロトタイプ宣言
Node *expr();
Node *mul();
Node *term();

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char  *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告するための関数
void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

// user_inputが指し示す文字列を
// トークンに分割してtokensに保存する
void tokenize() {
    char *p = user_input;

    int i = 0;
    while(*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    tokenize();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックして
    // 最初のmov命令を出力
    if (tokens[0].ty != TK_NUM)
        error_at(tokens[0].input, "数ではありません");
    printf("  mov rax, %d\n", tokens[0].val);

    // `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
    // アセンブリを出力
    int i = 1;
    while (tokens[i].ty != TK_EOF) {
        if (tokens[i].ty == '+') {
            i++;
            if (tokens[i].ty != TK_NUM)
                error_at(tokens[i].input, "数ではありません");
            printf("  add rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        if (tokens[i].ty == '-') {
            i++;
            if (tokens[i].ty != TK_NUM)
                error_at(tokens[i].input, "数ではありません");
            printf("  sub rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        error_at(tokens[i].input, "予期しないトークンです");
    }

    printf("  ret\n");
    return 0;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));

    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));

    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int consume(int ty) {
    if (tokens[pos].ty == ty)
        return 0;
    pos++;
    return 1;
}

Node *expr() {
    Node *node = mul();
    
    for (;;) {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *term() {
    if (consume('(')) {
        Node *node = expr();
        if (consume(')'))
            error_at(tokens[pos].input, "開きカッコに対応する閉じカッコがありません");
        return node;
    }

    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos].val);
    
    error_at(tokens[pos].input, "数値でも閉じカッコでもないトークンです");
}
Node *mul() {
    Node *node = term();

    for (;;) {
        if (consume('*')) 
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
    }
}
