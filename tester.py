#! /usr/bin/env python3

from os import system as syscall
from string import ascii_lowercase, ascii_uppercase
alphabet = (ascii_lowercase + ascii_uppercase).replace('v', '')

TARGET = './petri_net'


class Tree:

    def __init__(self, sym: str, left=None, right=None):
        if sym == '>':
            self.sym = 'v'
            self.left = ~left
            self.right = right
        elif sym == '<':
            self.sym = 'v'
            self.left = left
            self.right = ~right
        elif sym == '=':
            self.sym = '^'
            self.left = Tree('>', left, right)
            self.right = Tree('<', right, left)
        elif sym in 'v^':
            self.sym = sym
            self.left = left
            self.right = right
        else:
            self.sym = sym
            self.left = None
            self.right = None

    def __and__(self, other) -> object:
        return Tree('^', self, other)

    def __or__(self, other) -> object:
        return Tree('v', self, other)

    def __invert__(self):
        if self.left is not None:
            return Tree('^' if self.sym == 'v' else 'v', ~self.left, ~self.right)
        else:
            return Tree('~' + self.sym if self.sym[0] != '~' else self.sym[1:])

    def __ge__(self, other):
        return Tree('>', self, other)

    def __le__(self, other):
        return Tree('<', self, other)

    def __eq__(self, other):
        return Tree('=', self, other)

    def __str__(self) -> str:
        if self.left is not None and self.right is not None:
            return '(%s%s%s)' % (self.left, self.sym, self.right)
        else:
            return self.sym


class Generator:

    @classmethod
    def methods(cls) -> list:
        return ['atomic_conj', 'impl_chain', 'impl_tree', 'expr_neg']

    @classmethod
    def atomic_conj(cls, size, alphabet=alphabet):
        expr = Tree('T')
        for letter, _ in zip(alphabet, range(size)):
            expr = (Tree(letter) | ~Tree(letter)) & expr
        return expr

    @classmethod
    def impl_chain(cls, size, alphabet=alphabet):
        expr = Tree(alphabet[size], None, None)
        for letter, previous, _ in zip(alphabet[1:], alphabet, range(size)):
            expr = (Tree(letter) >= Tree(previous)) & expr
        expr = expr >= Tree(alphabet[0])
        return expr

    @classmethod
    def impl_tree(cls, size, alphabet=alphabet):
        def __impl_tree(size, alphabet=alphabet):
            expr = Tree('T')
            if size > 1:
                expr = Tree(alphabet[0]) >= (Tree(alphabet[1]) & Tree(alphabet[2]))
            if size > 2:
                sub = __impl_tree(size//2, alphabet[1]+alphabet[3:]) & \
                      __impl_tree(size//2 + size%2,alphabet[2]+alphabet[size//2+2:])
                expr = sub & expr
            return expr
        expr = __impl_tree(size, alphabet) >= (Tree(alphabet[0]) >= Tree(alphabet[size]))
        return expr

    @classmethod
    def expr_neg(cls, size, alphabet=alphabet):
        expr = Tree('T')
        for letter, i in zip(alphabet, range(size)):
            expr = (Tree(letter) | expr) if i % 2 else (Tree(letter) & expr)
        expr = expr  | ~expr
        return expr

    @classmethod
    def generate(cls, method: str, size: int, alphabet: str = alphabet) -> Tree:
        if method in cls.methods():
            return getattr(cls, method)(size, alphabet)
        else:
            return Tree('F', None, None)


def test(expr) -> int:
    return syscall('%s "%s"' % (TARGET, expr)) // 256


def testsuite() -> dict:
    results: dict = {}
    for method in Generator.methods():
        results[method] = {}
        print(method, ':')
        for size in range(1, 21):
            expr = Generator.generate(method, size)
            print(expr)
            results[method][size] = test(expr)
            print()
        print('\n')
    return results


if __name__ == '__main__':
    results = testsuite()
    breakpoint()
