from PIL import Image

def text_to_bits(text):
    return ''.join(format(ord(c), '08b') for c in text)

def bits_to_text(bits):
    chars = [bits[i:i+8] for i in range(0, len(bits), 8)]
    return ''.join(chr(int(b, 2)) for b in chars)

def embed_message(image_path, message, output_path):
    img = Image.open(image_path)
    img = img.convert('RGB')
    message_bits = text_to_bits(message) + '1111111111111110'
    data_index = 0

    pixels = list(img.getdata())
    new_pixels = []

    for pixel in pixels:
        if data_index < len(message_bits):
            r, g, b = pixel[:3]
            r = (r & ~1) | int(message_bits[data_index])
            data_index += 1
            if data_index < len(message_bits):
                g = (g & ~1) | int(message_bits[data_index])
                data_index += 1
            if data_index < len(message_bits):
                b = (b & ~1) | int(message_bits[data_index])
                data_index += 1
            new_pixels.append((r, g, b))
        else:
            new_pixels.append(pixel)

    img.putdata(new_pixels)
    img.save(output_path)
    print("Сообщение внедрено")

def extract_message(image_path):
    img = Image.open(image_path)
    img = img.convert('RGB')
    pixels = list(img.getdata())

    binary_message = ""
    for pixel in pixels:
        r, g, b = pixel
        binary_message += str(r & 1)
        binary_message += str(g & 1)
        binary_message += str(b & 1)

    binary_message = binary_message.split('1111111111111110')[0]
    return bits_to_text(binary_message)

    message = ""
    for c in chars:
        ch = chr(int(c, 2))
        if ch == chr(0):
            break
        message += ch
    return message

if __name__ == "__main__":
    input_image = "input.png"
    output_image = "output.png"
    
    message = input("Cообщение для внедрения: ")
    embed_message(input_image, message, output_image)
    
    extracted = extract_message(output_image)
    print("Извлечённое сообщение:", extracted)
