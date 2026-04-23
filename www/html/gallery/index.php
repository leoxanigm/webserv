<?php
declare(strict_types=1);

$uploadDir = __DIR__ . '/uploads';
$allowedExts = array('jpg', 'jpeg', 'png', 'gif', 'webp');
$maxFileSize = 5 * 1024 * 1024; // 5 MB

if (!is_dir($uploadDir)) {
    mkdir($uploadDir, 0755, true);
}

function h($str)
{
    return htmlspecialchars((string)$str, ENT_QUOTES, 'UTF-8');
}

function redirectToSelf()
{
    header('Status: 303 See Other');
    header('Location: /gallery');
    exit;
}

function safeFilename($name)
{
    $name = basename($name);
    $name = preg_replace('/[^A-Za-z0-9._-]/', '_', $name);
    return $name;
}

function uniqueUploadName($dir, $filename)
{
    $pathInfo = pathinfo($filename);
    $base = isset($pathInfo['filename']) ? $pathInfo['filename'] : 'file';
    $ext = isset($pathInfo['extension']) ? '.' . $pathInfo['extension'] : '';

    $candidate = $base . $ext;
    $counter = 1;

    while (file_exists($dir . '/' . $candidate)) {
        $candidate = $base . '_' . $counter . $ext;
        $counter++;
    }

    return $candidate;
}

$message = '';
$error = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Delete image
    if (isset($_POST['delete_name'])) {
        $name = safeFilename($_POST['delete_name']);
        $fullPath = $uploadDir . '/' . $name;

        if ($name !== '' && is_file($fullPath)) {
            unlink($fullPath);
        }

        redirectToSelf();
    }

    // Upload image
    if (isset($_FILES['image'])) {
        $file = $_FILES['image'];

        if (!isset($file['error']) || is_array($file['error'])) {
            $error = 'Invalid upload.';
        } elseif ($file['error'] !== UPLOAD_ERR_OK) {
            $error = 'Upload failed with error code ' . (string)$file['error'] . '.';
        } elseif ((int)$file['size'] > $maxFileSize) {
            $error = 'File too large.';
        } else {
            $originalName = isset($file['name']) ? $file['name'] : '';
            $safeName = safeFilename($originalName);
            $ext = strtolower(pathinfo($safeName, PATHINFO_EXTENSION));

            if ($safeName === '' || !in_array($ext, $allowedExts, true)) {
                $error = 'Invalid file type.';
            } else {
                $finalName = uniqueUploadName($uploadDir, $safeName);
                $dest = $uploadDir . '/' . $finalName;

                if (!move_uploaded_file($file['tmp_name'], $dest)) {
                    // Fallback for environments where move_uploaded_file may not behave as expected
                    if (!rename($file['tmp_name'], $dest)) {
                        $error = 'Could not save uploaded file.';
                    }
                }

                if ($error === '') {
                    chmod($dest, 0644);
                    redirectToSelf();
                }
            }
        }
    }
}

// Build image list
$images = array();
$dirEntries = scandir($uploadDir);
if ($dirEntries !== false) {
    foreach ($dirEntries as $entry) {
        if ($entry === '.' || $entry === '..') {
            continue;
        }

        $fullPath = $uploadDir . '/' . $entry;
        if (!is_file($fullPath)) {
            continue;
        }

        $ext = strtolower(pathinfo($entry, PATHINFO_EXTENSION));
        if (!in_array($ext, $allowedExts, true)) {
            continue;
        }

        $images[] = array(
            'name' => $entry,
            'size' => filesize($fullPath),
            'url'  => '/gallery/uploads/' . rawurlencode($entry),
        );
    }
}

header('Content-Type: text/html; charset=UTF-8');
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Gallery</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            background: #f4f4f4;
            color: #222;
        }
        h1, h2 {
            margin-bottom: 16px;
        }
        .box {
            background: #fff;
            border: 1px solid #ddd;
            padding: 20px;
            margin-bottom: 24px;
        }
        .msg {
            padding: 12px;
            margin-bottom: 16px;
            border: 1px solid #ccc;
            background: #fafafa;
        }
        .err {
            border-color: #c33;
            background: #ffeaea;
        }
        .grid {
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
        }
        .card {
            width: 220px;
            background: #fff;
            border: 1px solid #ddd;
            padding: 12px;
        }
        .card img {
            display: block;
            width: 100%;
            height: 160px;
            object-fit: cover;
            border: 1px solid #ccc;
            background: #eee;
        }
        .meta {
            margin-top: 10px;
            word-break: break-all;
            font-size: 14px;
        }
        form {
            margin: 0;
        }
        .delete-btn {
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <h1>Gallery</h1>

    <div class="box">
        <h2>Upload image</h2>

        <?php if ($error !== ''): ?>
            <div class="msg err"><?php echo h($error); ?></div>
        <?php endif; ?>

        <form action="/gallery" method="POST" enctype="multipart/form-data">
            <input type="file" name="image" accept=".jpg,.jpeg,.png,.gif,.webp" required>
            <button type="submit">Upload</button>
        </form>
    </div>

    <div class="box">
        <h2>Images</h2>

        <?php if (count($images) === 0): ?>
            <p>No images uploaded.</p>
        <?php else: ?>
            <div class="grid">
                <?php foreach ($images as $img): ?>
                    <div class="card">
                        <img src="<?php echo h($img['url']); ?>" alt="<?php echo h($img['name']); ?>">
                        <div class="meta">
                            <div><strong><?php echo h($img['name']); ?></strong></div>
                            <div><?php echo (int)$img['size']; ?> bytes</div>
                        </div>
                        <form action="/gallery" method="POST" class="delete-btn">
                            <input type="hidden" name="delete_name" value="<?php echo h($img['name']); ?>">
                            <button type="submit">Delete</button>
                        </form>
                    </div>
                <?php endforeach; ?>
            </div>
        <?php endif; ?>
    </div>
</body>
</html>