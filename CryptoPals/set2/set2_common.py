from random import randint

class PKCS7PaddingError(Exception):
    def __init__(self, message, data):
        super(PKCS7PaddingError, self).__init__(message)
        self.data = data

    def __str__(self):
        return self.message + ": " + repr(self.data)

def pkcs7_padding(text, block_size=16):
    pad = len(text) % block_size
    if pad == 0:
        return text + chr(block_size) * block_size
    pad = block_size - pad
    return text + (chr(pad) * pad)

def pkcs7_unpad(text, block_size=16):
    pad = ord(text[-1])
    if pad > block_size:
        raise PKCS7PaddingError("Given padding is not valid", text)

    pad_chars = text[-pad:]
    if (len(pad_chars) == 1 and ord(pad_chars[0]) == 0x01) or \
        all([pad_chars[0] == c for c in pad_chars[1:]]):
        return text[:-pad]
    else:
        raise PKCS7PaddingError("Given padding is not valid", text)

def gen_aes_key(length=16):
    return "".join([chr(randint(0, 255)) for _ in range(length)])

def break_into_blocks(text, block_size=16):
    return [text[i:i + block_size] for i in range(0, len(text), block_size)]

def similar_blocks(text, block_size=16):
    blocks = break_into_blocks(text, block_size)
    repeated_blocks = []
    for n, block1 in enumerate(blocks):
        for m, block2 in enumerate(blocks):
            if n != m and block1 == block2 and block1 not in repeated_blocks:
                repeated_blocks.append(block1)
    return repeated_blocks

def find_block_size(cipher):
    base_len = len(cipher("A"))
    block_length = 0
    for i in range(2, 512):
        test_len = len(cipher("A" * i))
        if test_len > base_len:
            if block_length:
                return block_length
            else:
                block_length = 1
                base_len = test_len
        elif block_length:
            block_length += 1

    return -1

def detect_ecb(cipher):
    break_str = "A"*16*3
    enc = cipher(break_str)
    return len(similar_blocks(enc)) > 0

def print_hex(h):
    print [hex(ord(c)) for c in h]
