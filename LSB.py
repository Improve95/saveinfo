from PIL import Image

def text_to_bits(text):
    """Преобразует строку в последовательность бит."""
    return ''.join(format(ord(c), '08b') for c in text)

def bits_to_text(bits):
    """Преобразует последовательность бит обратно в строку."""
    chars = [bits[i:i+8] for i in range(0, len(bits), 8)]
    return ''.join(chr(int(b, 2)) for b in chars)

def embed_message(image_path, message, output_path):
    """Встраивает сообщение в изображение."""
    img = Image.open(image_path)
    encoded = img.copy()
    width, height = img.size
    message += chr(0)  # символ конца сообщения
    message_bits = text_to_bits(message)
    data_index = 0

    pixels = list(img.getdata())
    new_pixels = []

    for pixel in pixels:
        if data_index < len(message_bits):
            r, g, b = pixel[:3]
            r = (r & ~1) | int(message_bits[data_index])      # бит в R
            data_index += 1
            if data_index < len(message_bits):
                g = (g & ~1) | int(message_bits[data_index])  # бит в G
                data_index += 1
            if data_index < len(message_bits):
                b = (b & ~1) | int(message_bits[data_index])  # бит в B
                data_index += 1
            new_pixels.append((r, g, b))
        else:
            new_pixels.append(pixel)

    encoded.putdata(new_pixels)
    encoded.save(output_path)
    print(f"✅ Сообщение внедрено и сохранено как {output_path}")

def extract_message(image_path):
    """Извлекает сообщение из изображения."""
    img = Image.open(image_path)
    bits = ""
    for pixel in img.getdata():
        for color in pixel[:3]:
            bits += str(color & 1)
    chars = [bits[i:i+8] for i in range(0, len(bits), 8)]

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
    
    message = input("Введите сообщение для внедрения: ")
    embed_message(input_image, message, output_image)
    
    extracted = extract_message(output_image)
    print("📩 Извлечённое сообщение:", extracted)
