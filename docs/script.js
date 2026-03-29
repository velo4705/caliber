// Scroll reveal animation
const observer = new IntersectionObserver((entries) => {
    entries.forEach(e => {
        if (e.isIntersecting) e.target.classList.add('visible');
    });
}, { threshold: 0.1 });
document.querySelectorAll('.reveal').forEach(el => observer.observe(el));

// Smooth scroll for nav links
document.querySelectorAll('a[href^="#"]').forEach(a => {
    a.addEventListener('click', e => {
        e.preventDefault();
        const target = document.querySelector(a.getAttribute('href'));
        if (target) target.scrollIntoView({ behavior: 'smooth' });
    });
});

// Floating math symbols animation
(function() {
    const symbols = [
        '∫', '∑', 'π', '√', '∞', '∂', 'Δ', 'θ', 'λ', 'σ',
        'x²', 'y³', 'f(x)', 'dy/dx', 'sin', 'cos', 'tan',
        'log', 'lim', '∀', '∃', '∈', '∇', '⊗', '⊕',
        'A×B', 'det(A)', 'e^x', 'ln', 'Φ', 'Ω', 'μ', 'ε',
        'R²', 'R³', 'n!', 'C(n,k)', 'P(A)', '∑xi', 'μx'
    ];
    const container = document.getElementById('mathParticles');
    if (!container) return;

    function createParticle() {
        const el = document.createElement('span');
        el.className = 'math-particle';
        el.textContent = symbols[Math.floor(Math.random() * symbols.length)];
        el.style.left = Math.random() * 100 + '%';
        el.style.fontSize = (14 + Math.random() * 20) + 'px';
        el.style.animationDuration = (15 + Math.random() * 25) + 's';
        el.style.animationDelay = Math.random() * 5 + 's';
        container.appendChild(el);

        // Remove after animation completes
        const lifetime = (parseFloat(el.style.animationDuration) + parseFloat(el.style.animationDelay)) * 1000;
        setTimeout(() => el.remove(), lifetime);
    }

    // Spawn particles periodically
    for (let i = 0; i < 15; i++) createParticle();
    setInterval(() => { if (container.children.length < 25) createParticle(); }, 2000);
})();
