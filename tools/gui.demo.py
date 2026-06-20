#!/usr/bin/env python3

from nicegui import ui


def root():

    async def send():

        question = text.value.strip()

        if not question:
            return

        text.value = ''

        with message_container:
            ui.chat_message(
                text=question,
                name='You',
                sent=True
            )

            ui.chat_message(
                text=f'Fake response to: "{question}"',
                name='Burghy',
                sent=False
            )

        ui.run_javascript(
            'window.scrollTo(0, document.body.scrollHeight)'
        )

    message_container = (
        ui.column()
        .classes('w-full max-w-2xl mx-auto flex-grow items-stretch')
    )

    with ui.footer().classes('bg-white'):
        with ui.row().classes('w-full no-wrap items-center'):

            text = (
                ui.input(
                    placeholder='Write a message...'
                )
                .classes('w-full')
                .on('keydown.enter', send)
            )

            ui.button(
                'Send',
                on_click=send
            )


ui.run(
    root,
    title='Burghy Chat'
)