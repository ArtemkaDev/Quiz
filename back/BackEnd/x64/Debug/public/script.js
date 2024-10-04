document.addEventListener('DOMContentLoaded', () => {
    const startScreen = document.getElementById('start-screen');
    const menuScreen = document.getElementById('menu-screen');
    const categoryScreen = document.getElementById('category-screen');
    const quizScreen = document.getElementById('quiz-screen');
    const resultScreen = document.getElementById('result-screen');

    const usernameInput = document.getElementById('username');
    const userSpan = document.getElementById('user');
    const questionElement = document.getElementById('question');
    const optionsElement = document.getElementById('options');
    const scoreElement = document.getElementById('score');

    let currentQuestionIndex = 0;
    let score = 0;
    let selectedCategory = '';
    let quizQuestions = [];

    const questions = {
        history: [
            { question: 'Когда началась Первая мировая война?', options: ['1914', '1918', '1939', '1945'], correct: 0 },
            { question: 'Кто был первым президентом США?', options: ['Авраам Линкольн', 'Джон Адамс', 'Джордж Вашингтон', 'Томас Джефферсон'], correct: 2 },

        ],
        geography: [
            { question: 'Какая самая большая страна в мире?', options: ['Канада', 'Россия', 'Китай', 'США'], correct: 1 },
            { question: 'Какой океан самый большой?', options: ['Атлантический', 'Индийский', 'Северный Ледовитый', 'Тихий'], correct: 3 },

        ],
        biology: [
            { question: 'Что является основным органом кровообращения в теле человека?', options: ['Мозг', 'Печень', 'Сердце', 'Почки'], correct: 2 },
            { question: 'Какая наука изучает растения?', options: ['Зоология', 'Микробиология', 'Ботаника', 'Экология'], correct: 2 },

        ],
        math: [
            { question: 'Чему равен квадратный корень из 144?', options: ['12', '14', '16', '18'], correct: 0 },
            { question: 'Сколько граней у куба?', options: ['4', '6', '8', '12'], correct: 1 },

        ]
    };

    document.getElementById('start-btn').addEventListener('click', () => {
        const username = usernameInput.value.trim();
        if (username) {
            userSpan.textContent = username;
            startScreen.classList.add('d-none');
            menuScreen.classList.remove('d-none');
        } else {
            alert('Введите ваше имя');
        }
    });

    document.getElementById('new-quiz-btn').addEventListener('click', () => {
        menuScreen.classList.add('d-none');
        categoryScreen.classList.remove('d-none');
    });

    document.getElementById('exit-btn').addEventListener('click', () => {
        menuScreen.classList.add('d-none');
        startScreen.classList.remove('d-none');
        usernameInput.value = '';
    });

    document.querySelectorAll('.category-btn').forEach(button => {
        button.addEventListener('click', () => {
            selectedCategory = button.getAttribute('data-category');
            quizQuestions = questions[selectedCategory];
            currentQuestionIndex = 0;
            score = 0;
            categoryScreen.classList.add('d-none');
            quizScreen.classList.remove('d-none');
            showQuestion();
        });
    });

    document.getElementById('next-btn').addEventListener('click', () => {
        currentQuestionIndex++;
        if (currentQuestionIndex < quizQuestions.length) {
            showQuestion();
        } else {
            showResults();
        }
    });

    function showQuestion() {
        const currentQuestion = quizQuestions[currentQuestionIndex];
        questionElement.textContent = currentQuestion.question;
        optionsElement.innerHTML = '';

        currentQuestion.options.forEach((option, index) => {
            const optionButton = document.createElement('button');
            optionButton.textContent = option;
            optionButton.classList.add('list-group-item', 'list-group-item-action');
            optionButton.addEventListener('click', () => {
                if (index === currentQuestion.correct) {
                    score++;
                }
                document.getElementById('next-btn').classList.remove('d-none');
            });
            optionsElement.appendChild(optionButton);
        });

        document.getElementById('next-btn').classList.add('d-none');
    }

    function showResults() {
        quizScreen.classList.add('d-none');
        resultScreen.classList.remove('d-none');
        scoreElement.textContent = `Вы ответили правильно на ${score} из ${quizQuestions.length} вопросов.`;
    }

    document.getElementById('back-to-menu-btn').addEventListener('click', () => {
        resultScreen.classList.add('d-none');
        menuScreen.classList.remove('d-none');
    });
});
