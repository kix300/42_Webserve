<?php
// CGI PHP super simple pour Webserve
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI PHP - Webserve Test</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f2f5;
            color: #333;
            line-height: 1.6;
        }
        
        header {
            background: linear-gradient(135deg, #6e8efb, #a777e3);
            color: white;
            padding: 2rem 0;
            text-align: center;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        
        h1 {
            margin: 0;
            font-size: 2.5rem;
        }
        
        .container {
            max-width: 1200px;
            margin: 2rem auto;
            padding: 0 2rem;
        }
        
        .card {
            background: white;
            border-radius: 8px;
            padding: 1.5rem;
            margin-bottom: 2rem;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease;
        }
        
        .btn {
            display: inline-block;
            background: #6e8efb;
            color: white;
            padding: 0.7rem 1.5rem;
            text-decoration: none;
            border-radius: 50px;
            transition: background 0.3s ease;
            margin: 5px;
        }
        
        .btn:hover {
            background: #5a7bf0;
        }

        .info-box {
            background: #f8f9fa;
            border-left: 4px solid #6e8efb;
            padding: 1rem;
            margin: 1rem 0;
            border-radius: 4px;
        }

        .result-box {
            background: #e8f5e8;
            border-left: 4px solid #4caf50;
            padding: 1rem;
            margin: 1rem 0;
            border-radius: 4px;
        }

        form {
            background: #f8f9fa;
            padding: 1.5rem;
            border-radius: 8px;
            margin: 1rem 0;
        }

        input, textarea, select {
            width: 100%;
            padding: 0.7rem;
            margin: 0.5rem 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-family: inherit;
        }

        button {
            background: #6e8efb;
            color: white;
            padding: 0.7rem 1.5rem;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            transition: background 0.3s ease;
        }

        button:hover {
            background: #5a7bf0;
        }

        @media (max-width: 768px) {
            h1 {
                font-size: 2rem;
            }
        }
    </style>
</head>
<body>
    <header>
        <div class="container">
            <h1>CGI PHP - Webserve Test</h1>
        </div>
    </header>
    
    <div class="container">
        <!-- Navigation principale -->
        <div class="card">
            <h2>Navigation</h2>
            <p>Explorez les différentes fonctionnalités du serveur</p>
            <a href="/" class="btn">Accueil</a>
            <a href="/methode/" class="btn">Test des Méthodes</a>
            <a href="/test3/" class="btn">Test de redirection serveur</a>
            <a href="/files/" class="btn">Test de autoindex</a>
            <a href="/cgi/form.py" class="btn">Test du formulaire CGI Python</a>
            <a href="/cgi/timeout.py" class="btn">Test de timeout CGI</a>
        </div>

        <!-- Informations du serveur -->
        <div class="card">
            <h2>Informations du serveur</h2>
            <div class="info-box">
                <strong>Date/Heure:</strong> <?php echo date('Y-m-d H:i:s'); ?><br>
                <strong>Méthode HTTP:</strong> <?php echo $_SERVER['REQUEST_METHOD'] ?? 'N/A'; ?><br>
                <strong>Script:</strong> <?php echo $_SERVER['SCRIPT_NAME'] ?? 'N/A'; ?><br>
                <strong>Query String:</strong> <?php echo $_SERVER['QUERY_STRING'] ?? '(vide)'; ?><br>
                <strong>User Agent:</strong> <?php echo substr($_SERVER['HTTP_USER_AGENT'] ?? 'N/A', 0, 100); ?>...
            </div>
        </div>

        <?php if (!empty($_GET)): ?>
        <div class="card">
            <h2>Paramètres GET reçus</h2>
            <div class="result-box">
                <?php foreach ($_GET as $key => $value): ?>
                    <strong><?php echo htmlspecialchars($key); ?>:</strong> 
                    <?php echo htmlspecialchars($value); ?><br>
                <?php endforeach; ?>
            </div>
        </div>
        <?php endif; ?>

        <?php if (!empty($_POST)): ?>
        <div class="card">
            <h2>Données POST reçues</h2>
            <div class="result-box">
                <?php foreach ($_POST as $key => $value): ?>
                    <strong><?php echo htmlspecialchars($key); ?>:</strong> 
                    <?php echo htmlspecialchars($value); ?><br>
                <?php endforeach; ?>
            </div>
        </div>
        <?php endif; ?>

        <!-- Formulaires de test -->
        <div class="card">
            <h2>Formulaires de test</h2>
            <p>Testez les requêtes GET et POST avec ce script PHP</p>
            
            <h3>Test GET</h3>
            <form method="GET">
                <input type="text" name="test_get" placeholder="Entrez du texte pour test GET" value="<?php echo htmlspecialchars($_GET['test_get'] ?? ''); ?>">
                <button type="submit">Envoyer GET</button>
            </form>
            
            <h3>Test POST</h3>
            <form method="POST">
                <input type="text" name="nom" placeholder="Votre nom" value="<?php echo htmlspecialchars($_POST['nom'] ?? ''); ?>">
                <input type="email" name="email" placeholder="Votre email" value="<?php echo htmlspecialchars($_POST['email'] ?? ''); ?>">
                <textarea name="message" placeholder="Votre message"><?php echo htmlspecialchars($_POST['message'] ?? ''); ?></textarea>
                <button type="submit">Envoyer POST</button>
            </form>
        </div>

        <!-- Informations techniques -->
        <div class="card">
            <h2>Informations techniques PHP</h2>
            <div class="info-box">
                <strong>Version PHP:</strong> <?php echo phpversion(); ?><br>
                <strong>SAPI:</strong> <?php echo php_sapi_name(); ?><br>
                <strong>Mémoire utilisée:</strong> <?php echo round(memory_get_usage()/1024/1024, 2); ?> MB<br>
                <strong>Limite mémoire:</strong> <?php echo ini_get('memory_limit'); ?><br>
                <strong>Limite d'exécution:</strong> <?php echo ini_get('max_execution_time'); ?>s
            </div>
        </div>
    </div>
</body>
</html>
